#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include "include/vlc/vlc.h"

#define APP L"NDI PLAY"
#define CLASS L"NDIPlayPortableTray_030"
#define TRAYMSG (WM_APP+1)
#define COPYTAG 0x4E445030
#define MAX_PLAYERS 256
static HWND window;
static HINSTANCE app;
static HANDLE job;
static NOTIFYICONDATAW tray;
static wchar_t executable[32768],vlcFolder[32768],tempFolder[MAX_PATH];
static UINT taskbarCreated;
typedef struct { HANDLE process,stop; DWORD pid; wchar_t *file; wchar_t log[MAX_PATH],error[MAX_PATH]; ULONGLONG stopDeadline; } Child;
static Child children[MAX_PLAYERS];
static unsigned childCount,serial;
static int closing,failed,errorShown;

static char *utf8(const wchar_t *text) {
 int n=WideCharToMultiByte(CP_UTF8,0,text,-1,NULL,0,NULL,NULL);
 char *out=malloc((size_t)n);if(out)WideCharToMultiByte(CP_UTF8,0,text,-1,out,n,NULL,NULL);return out;
}
static wchar_t *wide(const char *text) {
 int n=MultiByteToWideChar(CP_UTF8,0,text,-1,NULL,0);
 wchar_t *out=calloc((size_t)n,sizeof(wchar_t));if(out)MultiByteToWideChar(CP_UTF8,0,text,-1,out,n);return out;
}
/* Windows CommandLineToArgvW rules, including embedded quotes and terminal \\. */
static wchar_t *quote(const wchar_t *text) {
 size_t len=wcslen(text),pos=0;wchar_t *out=calloc(len*2+3,sizeof(wchar_t));if(!out)return NULL;
 out[pos++]=L'"';
 for(size_t i=0;i<len;){
  size_t slashes=0;while(i<len&&text[i]==L'\\'){slashes++;i++;}
  size_t copies=slashes*((i==len||text[i]==L'"')?2:1);
  while(copies--)out[pos++]=L'\\';
  if(i<len){if(text[i]==L'"')out[pos++]=L'\\';out[pos++]=text[i++];}
 }
 out[pos++]=L'"';out[pos]=0;return out;
}
static void errorText(const wchar_t *text){MessageBoxW(window,text,APP,MB_OK|MB_ICONERROR);}
static int containsNdi(const char *s){
 for(;*s;s++)if((s[0]=='N'||s[0]=='n')&&(s[1]=='D'||s[1]=='d')&&(s[2]=='I'||s[2]=='i'))return 1;
 return 0;
}
static int isFile(const wchar_t *path){DWORD a=GetFileAttributesW(path);return a!=INVALID_FILE_ATTRIBUTES&&!(a&FILE_ATTRIBUTE_DIRECTORY);}
static int checkFolder(const wchar_t *path){
 wchar_t dll[32768];if(wcslen(path)>32000)return 0;
 swprintf(dll,32768,L"%ls\\libvlc.dll",path);if(!isFile(dll))return 0;
 wcscpy(vlcFolder,path);return 1;
}
static int findVlc(void){
 wchar_t path[32768],base[32768];DWORD size=sizeof(path);
 if(RegGetValueW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\VideoLAN\\VLC",L"InstallDir",RRF_RT_REG_SZ|RRF_SUBKEY_WOW6464KEY,NULL,path,&size)==ERROR_SUCCESS&&checkFolder(path))return 1;
 DWORD n=GetEnvironmentVariableW(L"ProgramW6432",base,32768);if(!n)n=GetEnvironmentVariableW(L"ProgramFiles",base,32768);
 if(n&&n<32000){swprintf(path,32768,L"%ls\\VideoLAN\\VLC",base);if(checkFolder(path))return 1;}
 size=sizeof(path);
 if(RegGetValueW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\vlc.exe",NULL,RRF_RT_REG_SZ|RRF_SUBKEY_WOW6464KEY,NULL,path,&size)==ERROR_SUCCESS){wchar_t *p=wcsrchr(path,L'\\');if(p){*p=0;if(checkFolder(path))return 1;}}
 return 0;
}
static void tooltip(void){
 swprintf(tray.szTip,128,L"NDI PLAY — %u video%s. Right-click: Remove source / Close",childCount,childCount==1?L"":L"s");Shell_NotifyIconW(NIM_MODIFY,&tray);
}
static void removeChild(unsigned i){
 CloseHandle(children[i].process);CloseHandle(children[i].stop);free(children[i].file);
 DeleteFileW(children[i].error);
 if(!failed)DeleteFileW(children[i].log);
 children[i]=children[--childCount];tooltip();
}
static void startFile(const wchar_t *path){
 if(closing)return;
 if(!isFile(path)){wchar_t msg[33000];swprintf(msg,33000,L"File not found:\n%ls",path);errorText(msg);return;}
 if(childCount==MAX_PLAYERS){errorText(L"Too many players are already running.");return;}
 Child c={0};wchar_t eventName[128];serial++;
 swprintf(eventName,128,L"Local\\NDIPlayStop_%lu_%u",GetCurrentProcessId(),serial);
 c.stop=CreateEventW(NULL,TRUE,FALSE,eventName);if(!c.stop){errorText(L"Could not create the player stop event.");return;}
 swprintf(c.log,MAX_PATH,L"%ls\\player-%u.log",tempFolder,serial);
 swprintf(c.error,MAX_PATH,L"%ls\\player-%u.error",tempFolder,serial);
 const wchar_t *values[]={executable,L"--worker",vlcFolder,path,eventName,c.log,c.error};
 wchar_t *command=calloc(32768,sizeof(wchar_t));size_t used=0;int valid=command!=NULL;
 for(unsigned i=0;i<7&&valid;i++){
  wchar_t *q=quote(values[i]);size_t n=q?wcslen(q):32768;
  if(used+n+2>=32768){valid=0;free(q);break;}
  if(i)command[used++]=L' ';memcpy(command+used,q,n*sizeof(wchar_t));used+=n;command[used]=0;free(q);
 }
 if(!valid){free(command);CloseHandle(c.stop);errorText(L"The file path is too long to start a player.");return;}
 STARTUPINFOW si={0};si.cb=sizeof(si);si.dwFlags=STARTF_USESHOWWINDOW;si.wShowWindow=SW_HIDE;PROCESS_INFORMATION pi={0};
 BOOL started=CreateProcessW(executable,command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&si,&pi);free(command);
 if(!started){CloseHandle(c.stop);errorText(L"Could not start the VLC media worker.");return;}
 if(!AssignProcessToJobObject(job,pi.hProcess)){TerminateProcess(pi.hProcess,1);CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(c.stop);errorText(L"Could not attach the player to the application. No player was left running.");return;}
 c.process=pi.hProcess;c.pid=pi.dwProcessId;c.file=_wcsdup(path);
 children[childCount++]=c;ResumeThread(pi.hThread);CloseHandle(pi.hThread);tooltip();
}
static void checkChildren(void){
 for(unsigned i=0;i<childCount;){
  if(WaitForSingleObject(children[i].process,0)!=WAIT_OBJECT_0){
   if(children[i].stopDeadline && GetTickCount64()>=children[i].stopDeadline){
    if(TerminateProcess(children[i].process,0))children[i].stopDeadline=GetTickCount64()+1000;
   }
   i++;continue;
  }
  DWORD code=0;GetExitCodeProcess(children[i].process,&code);
  if(!closing&&!children[i].stopDeadline&&code){
   wchar_t message[4096],detail[2048]=L"";FILE *f=_wfopen(children[i].error,L"rb");
   if(f){char text[4096];size_t n=fread(text,1,sizeof(text)-1,f);text[n]=0;fclose(f);wchar_t *w=wide(text);if(w){wcsncpy(detail,w,2047);free(w);}}
   const wchar_t *file=wcsrchr(children[i].file,L'\\');file=file?file+1:children[i].file;
   swprintf(message,4096,L"%ls\n\n%ls\n\nDetails:\n%ls",file,*detail?detail:L"VLC media worker stopped unexpectedly.",children[i].log);
   failed=1;
   if(!errorShown){errorShown=1;errorText(message);}

  }
  removeChild(i);
 }
}
static LRESULT CALLBACK wndproc(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
 if(msg==taskbarCreated){Shell_NotifyIconW(NIM_ADD,&tray);return 0;}
 switch(msg){
 case WM_COPYDATA:{
  errorShown=0;
  COPYDATASTRUCT *data=(COPYDATASTRUCT*)lp;
  if(!data->lpData||data->dwData!=COPYTAG||data->cbData<4||data->cbData>1024*1024||data->cbData%sizeof(wchar_t))return FALSE;
  wchar_t *p=data->lpData;size_t left=data->cbData/sizeof(wchar_t);
  while(left&&*p){size_t n=0;while(n<left&&p[n])n++;if(n==left)return FALSE;startFile(p);p+=n+1;left-=n+1;}
  return TRUE;
 }
 case WM_TIMER:checkChildren();return 0;
 case TRAYMSG:
  if(lp==WM_RBUTTONUP||lp==WM_CONTEXTMENU){
   checkChildren();
   HMENU menu=CreatePopupMenu(),sources=CreatePopupMenu();
   DWORD sourcePids[MAX_PLAYERS]; unsigned sourceCount=childCount;
   for(unsigned i=0;i<sourceCount;i++){
    sourcePids[i]=children[i].pid;
    const wchar_t *name=wcsrchr(children[i].file,L'\\');name=name?name+1:children[i].file;
    wchar_t label[1024];size_t pos=0;
    pos=(size_t)swprintf(label,1024,L"%u. ",i+1);
    for(size_t j=0;name[j]&&pos<990;j++){
     if(name[j]==L'&')label[pos++]=L'&';
     label[pos++]=name[j];
    }
    label[pos]=0;
    if(children[i].stopDeadline)wcscat(label,L" (stopping...)");
    AppendMenuW(sources,MF_STRING|(children[i].stopDeadline?MF_GRAYED:0),100+i,label);
   }
   if(!sourceCount)AppendMenuW(sources,MF_STRING|MF_GRAYED,0,L"No active sources");
   AppendMenuW(menu,MF_POPUP,(UINT_PTR)sources,L"Remove source");
   AppendMenuW(menu,MF_SEPARATOR,0,NULL);
   AppendMenuW(menu,MF_STRING,1,L"Close");POINT p;GetCursorPos(&p);SetForegroundWindow(hwnd);
   UINT cmd=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_NONOTIFY|TPM_RIGHTBUTTON,p.x,p.y,0,hwnd,NULL);
   DestroyMenu(menu);PostMessageW(hwnd,WM_NULL,0,0);
   if(cmd==1)PostMessageW(hwnd,WM_CLOSE,0,0);
   else if(cmd>=100&&cmd<100+sourceCount){
    DWORD pid=sourcePids[cmd-100];
    for(unsigned i=0;i<childCount;i++)if(children[i].pid==pid){
     if(!children[i].stopDeadline){SetEvent(children[i].stop);children[i].stopDeadline=GetTickCount64()+3000;}
     break;
    }
   }
  }return 0;
 case WM_CLOSE:
  closing=1;KillTimer(hwnd,1);Shell_NotifyIconW(NIM_DELETE,&tray);
  for(unsigned i=0;i<childCount;i++)SetEvent(children[i].stop);
  /* One overall graceful shutdown deadline, then the job closes all remaining owned workers. */
  {ULONGLONG deadline=GetTickCount64()+3000;
   for(unsigned i=0;i<childCount;i++){ULONGLONG now=GetTickCount64();WaitForSingleObject(children[i].process,now<deadline?(DWORD)(deadline-now):0);}}
  CloseHandle(job);job=NULL;
  while(childCount){WaitForSingleObject(children[0].process,1000);removeChild(0);}
  if(!failed)RemoveDirectoryW(tempFolder);
  DestroyWindow(hwnd);return 0;
 case WM_DESTROY:PostQuitMessage(0);return 0;
 }return DefWindowProcW(hwnd,msg,wp,lp);
}

/* Only media workers load VLC/plugins. Native crashes cannot kill the tray. */
typedef void* (*new_fn)(int,const char *const *);
typedef void* (*path_fn)(void*,const char*);
typedef void* (*ptr_fn)(void*);
typedef void (*release_fn)(void*);
typedef void (*option_fn)(void*,const char*);
typedef int (*play_fn)(void*);
typedef int (*output_fn)(void*,const char*);
typedef libvlc_audio_output_t Output;
typedef Output *(*outputs_fn)(void*);
typedef void (*log_callback)(void*,int,const void*,const char*,va_list);
typedef void (*logset_fn)(void*,log_callback,void*);
static FILE *workerLog;
static CRITICAL_SECTION logGate;
static void vlcLog(void *data,int level,const void *context,const char *format,va_list args){
 (void)data;(void)context;EnterCriticalSection(&logGate);
 if(workerLog&&ftell(workerLog)<512*1024){fprintf(workerLog,"VLC level %d: ",level);vfprintf(workerLog,format,args);fputc('\n',workerLog);fflush(workerLog);}LeaveCriticalSection(&logGate);
}
static int workerFail(const wchar_t *errorPath,const char *message){FILE *f=_wfopen(errorPath,L"wb");if(f){fputs(message,f);fclose(f);}if(workerLog){fputs(message,workerLog);fflush(workerLog);}return 1;}
#define FN(type,name) type name=(type)(uintptr_t)GetProcAddress(lib,#name)
static int worker(int argc,wchar_t **argv){
 if(argc!=7)return 2;
 InitializeCriticalSection(&logGate);workerLog=_wfopen(argv[5],L"wb");
 HANDLE stop=OpenEventW(SYNCHRONIZE,FALSE,argv[4]);if(!stop)return workerFail(argv[6],"The launcher stop event was not available.");
 wchar_t libraryPath[32768];swprintf(libraryPath,32768,L"%ls\\libvlc.dll",argv[2]);SetDllDirectoryW(argv[2]);
 HMODULE lib=LoadLibraryExW(libraryPath,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
 if(!lib){char message[256];snprintf(message,sizeof(message),"Could not load VLC x64 (Windows error %lu). This portable EXE requires installed VLC 3 x64.",GetLastError());return workerFail(argv[6],message);}
 FN(new_fn,libvlc_new);FN(path_fn,libvlc_media_new_path);FN(ptr_fn,libvlc_media_player_new_from_media);
 FN(release_fn,libvlc_media_release);FN(release_fn,libvlc_release);FN(release_fn,libvlc_media_player_release);FN(release_fn,libvlc_media_player_stop);
 FN(option_fn,libvlc_media_add_option);FN(play_fn,libvlc_media_player_play);FN(play_fn,libvlc_media_player_get_state);
 FN(output_fn,libvlc_audio_output_set);FN(outputs_fn,libvlc_audio_output_list_get);FN(release_fn,libvlc_audio_output_list_release);FN(logset_fn,libvlc_log_set);
 const char *(*version)(void)=(void*)(uintptr_t)GetProcAddress(lib,"libvlc_get_version");
 if(!libvlc_new||!libvlc_media_new_path||!libvlc_media_player_new_from_media||!libvlc_media_release||!libvlc_release||!libvlc_media_player_release||!libvlc_media_player_stop||!libvlc_media_add_option||!libvlc_media_player_play||!libvlc_media_player_get_state||!libvlc_audio_output_set||!libvlc_audio_output_list_get||!libvlc_audio_output_list_release||!libvlc_log_set||!version)return workerFail(argv[6],"The installed VLC library is missing a required API.");
 const char *ver=version();if(workerLog){fprintf(workerLog,"VLC %s\nFirst audio track; native channel layout; NDI official outputs.\n",ver);fflush(workerLog);}
 if(strncmp(ver,"3.",2))return workerFail(argv[6],"This build supports VLC 3 x64. A different major version was found.");
 const char *options[]={"--ignore-config","--plugins-cache","--plugins-scan","--vout=NDI,none","--aout=NDI,none","--audio-track=0","--no-video-title-show","--no-osd","--no-spu","--no-audio-time-stretch","--audio-replay-gain-mode=none","--audio-filter=","--verbose=2"};
 void *instance=libvlc_new(sizeof(options)/sizeof(options[0]),options);if(!instance)return workerFail(argv[6],"VLC initialization failed.");
 libvlc_log_set(instance,vlcLog,NULL);
 Output *list=libvlc_audio_output_list_get(instance);char ndiOutput[128]="";
 for(Output *o=list;o;o=o->p_next){if(workerLog)fprintf(workerLog,"Audio output: %s (%s)\n",o->psz_name,o->psz_description);if(containsNdi(o->psz_name)||containsNdi(o->psz_description)){strncpy(ndiOutput,o->psz_name,sizeof(ndiOutput)-1);}}
 libvlc_audio_output_list_release(list);
 if(!*ndiOutput){libvlc_release(instance);return workerFail(argv[6],"VLC did not load the official NDI audio output plugin. Install or repair the NDI VLC Plugin for this VLC x64 installation. This EXE cannot replace an incompatible or missing plugin.");}
 char *file=utf8(argv[3]);void *media=file?libvlc_media_new_path(instance,file):NULL;free(file);
 if(!media){libvlc_release(instance);return workerFail(argv[6],"VLC could not open the file.");}
 libvlc_media_add_option(media,":audio-track=0");
 /* VLC3.0.24 rejects negative input-repeat. Repeat in the same input to retain outputs. */
 libvlc_media_add_option(media,":input-repeat=2147483647");
 void *player=libvlc_media_player_new_from_media(media);libvlc_media_release(media);
 if(!player){libvlc_release(instance);return workerFail(argv[6],"VLC could not create the player.");}
 int failure=0;
 if(libvlc_audio_output_set(player,ndiOutput)||libvlc_media_player_play(player)){failure=workerFail(argv[6],"VLC could not start NDI playback.");}
 else {
  while(WaitForSingleObject(stop,100)==WAIT_TIMEOUT){
   int state=libvlc_media_player_get_state(player);
   if(state==7){failure=workerFail(argv[6],"VLC reported a playback error. The log contains native plugin and decoder details.");break;}
   if(state==6){libvlc_media_player_stop(player);if(libvlc_media_player_play(player)){failure=workerFail(argv[6],"VLC could not restart the loop.");break;}}
  }
 }
 libvlc_media_player_stop(player);libvlc_media_player_release(player);libvlc_release(instance);CloseHandle(stop);
 if(workerLog)fclose(workerLog);DeleteCriticalSection(&logGate);
 return failure;
}
int WINAPI wWinMain(HINSTANCE h,HINSTANCE previous,wchar_t *command,int show){
 (void)previous;(void)command;(void)show;app=h;
 int argc=0;wchar_t **argv=CommandLineToArgvW(GetCommandLineW(),&argc);if(!argv)return 2;
 if(argc>1&&!wcscmp(argv[1],L"--worker")){int r=worker(argc,argv);LocalFree(argv);return r;}
 if(argc==3&&!wcscmp(argv[1],L"--extract-source")){
  HRSRC resource=FindResourceW(h,MAKEINTRESOURCEW(101),RT_RCDATA);HGLOBAL loaded=resource?LoadResource(h,resource):NULL;
  FILE *f=_wfopen(argv[2],L"wb");int r=1;
  if(loaded&&f){DWORD n=SizeofResource(h,resource);r=fwrite(LockResource(loaded),1,n,f)==n?0:1;}
  if(f)fclose(f);LocalFree(argv);return r;
 }
 HANDLE mutex=CreateMutexW(NULL,FALSE,L"Local\\NDIPlayPortableTray_030");if(!mutex)return 2;
 if(GetLastError()==ERROR_ALREADY_EXISTS){
  HWND other=NULL;for(unsigned n=0;n<40&&!other;n++){other=FindWindowW(CLASS,NULL);if(!other)Sleep(50);}
  if(other&&argc>1){size_t total=1;for(int i=1;i<argc;i++)total+=wcslen(argv[i])+1;
   wchar_t *data=calloc(total,sizeof(wchar_t)),*p=data;if(data){for(int i=1;i<argc;i++){wcscpy(p,argv[i]);p+=wcslen(p)+1;}
   COPYDATASTRUCT copy={COPYTAG,(DWORD)(total*sizeof(wchar_t)),data};DWORD_PTR result=0;
   if(!SendMessageTimeoutW(other,WM_COPYDATA,0,(LPARAM)&copy,SMTO_ABORTIFHUNG,5000,&result)||!result)errorText(L"The running NDI PLAY did not accept the files. Close it and retry.");free(data);}
  }else if(!other)errorText(L"NDI PLAY is starting or closing. Please retry in a moment.");
  LocalFree(argv);CloseHandle(mutex);return 0;
 }
 GetModuleFileNameW(NULL,executable,32768);
 if(!findVlc()){errorText(L"VLC 3 x64 was not found. Install VLC x64 and the official NDI VLC Plugin first.");LocalFree(argv);CloseHandle(mutex);return 1;}
 wchar_t base[MAX_PATH];GetTempPathW(MAX_PATH,base);swprintf(tempFolder,MAX_PATH,L"%lsNDI-PLAY-%lu-%llu",base,GetCurrentProcessId(),(unsigned long long)GetTickCount64());
 if(!CreateDirectoryW(tempFolder,NULL)){errorText(L"Could not create the diagnostic folder.");return 1;}
 job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
 if(!job||!SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits))){errorText(L"Could not create the player process group.");return 1;}
 WNDCLASSW cls={0};cls.lpfnWndProc=wndproc;cls.hInstance=h;cls.lpszClassName=CLASS;RegisterClassW(&cls);
 window=CreateWindowExW(0,CLASS,APP,WS_OVERLAPPED,0,0,0,0,NULL,NULL,h,NULL);if(!window){CloseHandle(job);return 1;}
 taskbarCreated=RegisterWindowMessageW(L"TaskbarCreated");
 tray.cbSize=sizeof(tray);tray.hWnd=window;tray.uID=1;tray.uFlags=NIF_MESSAGE|NIF_ICON|NIF_TIP;tray.uCallbackMessage=TRAYMSG;
 tray.hIcon=LoadIconW(h,MAKEINTRESOURCEW(1));wcscpy(tray.szTip,L"NDI PLAY — drop video files onto the EXE");
 if(!Shell_NotifyIconW(NIM_ADD,&tray)){errorText(L"Could not create the notification icon.");DestroyWindow(window);CloseHandle(job);return 1;}
 SetTimer(window,1,500,NULL);
 for(int i=1;i<argc;i++)startFile(argv[i]);LocalFree(argv);
 MSG msg;while(GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
 if(job)CloseHandle(job);CloseHandle(mutex);return 0;
}
