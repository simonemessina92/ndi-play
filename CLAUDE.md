# Repository instructions

## Language

Write all repository content in English, including documentation, code comments, commit messages, issues, and pull requests.

## Branches and releases

- Use `dev` for development and testing. Open pull requests against `dev`.
- Promote verified changes from `dev` to `main` only after Windows and NDI validation.
- Preserve the approved golden executable in `dist/` until a replacement has passed Windows and NDI validation. Do not replace it with an unverified build.

## Documentation

- Use direct, natural technical writing. Do not mention the project owner or private conversations in project documentation.
- Put a prominent direct EXE download link at the top of release notes.
- Keep README and release notes focused on features, requirements, downloads, and usage. Omit internal packaging history and comparisons with unpublished development builds. Keep build provenance in technical records only.

## Building

- See `docs/BUILD.md`. Regenerate `src/source.zip` with `python scripts/package_source.py` before building a changed executable, then run `src/build.cmd` with MSYS2 MinGW x64 `gcc` and `windres` on PATH.
- A successful compilation does not establish NDI functionality; see `docs/GOLDEN.md` for the validation checklist.
