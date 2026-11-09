# Integrate the refactor on Windows

Run the commands in PowerShell. Start inside your existing TimeTableX checkout. The baseline used for the package is `8b929d8e52ccd8e0b56882885bd2c7bbc4634573`.

## 1. Check the repository and create a separate worktree

```powershell
git status --short --branch
git remote -v
git fetch origin
git log -1 --oneline origin/main
git log -1 --oneline origin/feature/lab-scheduling
git log --oneline --left-right origin/main...origin/feature/lab-scheduling
git branch --list refactor/modular-oop
```

`git status` shows staged, unstaged, and untracked work. `git fetch` refreshes remote-tracking information. Keep any local edits in this checkout. The new worktree uses another directory, so your original edits stay where they are.

At packaging time, both remote branches pointed to the full SHA above. If either branch has advanced, review its new commits before using these instructions. If `refactor/modular-oop` already exists locally, inspect that branch before creating another checkout or applying anything.

```powershell
$base = '8b929d8e52ccd8e0b56882885bd2c7bbc4634573'
git worktree add -b refactor/modular-oop ..\TimeTableX-Refactor $base
if ($LASTEXITCODE -ne 0) { throw 'Worktree creation failed.' }
Set-Location ..\TimeTableX-Refactor
git status --short --branch
```

## 2. Bring in the files

Choose one route. Download the corresponding file into your Downloads folder or adjust the path.

### Route A. Apply the staged patch

```powershell
$patch = Join-Path $env:USERPROFILE 'Downloads\TimeTableX-OOP-Refactor.patch'
git am --3way $patch
if ($LASTEXITCODE -ne 0) { throw 'Patch application stopped. See git status.' }
git log --oneline "$base..HEAD"
```

`git am` replays the six actual development commits, preserving their recorded author and messages. Your Git identity is recorded as the committer. If it stops at a conflict, use `git status` to see what happened. In this fresh worktree, `git am --abort` returns to the state before the patch application.

### Route B. Copy the ZIP and stage the changes yourself

```powershell
$zip = Join-Path $env:USERPROFILE 'Downloads\TimeTableX-OOP-Refactor.zip'
$extract = Join-Path $env:TEMP ('TimeTableX-' + [guid]::NewGuid().ToString('N'))
Expand-Archive -LiteralPath $zip -DestinationPath $extract
$repoRoot = (Get-Location).Path
Get-ChildItem -LiteralPath $extract -Force | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination $repoRoot -Recurse -Force
}
Remove-Item -LiteralPath .\prototype\main.cpp, .\prototype\run_windows.bat, .\prototype\START_HERE.txt
git status --short
```

The three removals are the superseded source and launcher plus the original guide that has moved to `docs/archive/`. The ZIP contains the project source and historical materials. It is laid out to copy into the existing checkout root.

## 3. Build and verify on your laptop

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1
if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
.\build\timetablex.exe --test
if ($LASTEXITCODE -ne 0) { throw 'A self-test failed.' }
.\build\timetablex.exe --demo-file data/courses.csv
if ($LASTEXITCODE -ne 0) { throw 'CSV demo failed.' }
Import-Csv .\timetable.csv | Measure-Object
Import-Csv .\timetable.csv | Where-Object Course -eq 'C++ Lab'
.\build\timetablex.exe --demo
.\build\timetablex.exe --impossible
```

Look for `74 tests passed.`, nine exported rows, and the Sunday 14:00–16:00 C++ lab in Room-101. The impossible demo should print that search was exhausted. `build/` and the generated `timetable.csv` are ignored by Git.

## 4. Review and commit

For the patch route, the files are already committed. Use `git diff --stat $base HEAD`, `git diff --check $base HEAD`, and `git diff $base HEAD` to review the complete change.

For the ZIP route, review `git status --short` and use the explicit staging commands below. Git will show newly staged files in the cached diff.

```powershell
git add -- .gitignore CMakeLists.txt include src tests scripts prototype/main.cpp prototype/run_windows.bat
git diff --cached --stat
git diff --cached --check
git diff --cached
git commit -m "refactor: modularize TimeTableX and preserve its self-tests"

git add -- README.md docs learnings prototype/START_HERE.txt
git diff --cached --stat
git diff --cached --check
git diff --cached
git commit -m "docs: document architecture demo and integration"

git add -- .github/workflows/cpp.yml
git diff --cached --stat
git diff --cached --check
git diff --cached
git commit -m "ci: build TimeTableX and run its self-tests"
```

Review the first group as one buildable source change, the second as the documentation update, and the third as CI.

## 5. Push and review the pull request

```powershell
git status --short --branch
git log --oneline "$base..HEAD"
git diff --check $base HEAD
git push -u origin refactor/modular-oop
```

On GitHub, open a pull request with `main` as the base and `refactor/modular-oop` as the compare branch. Suggested title is **Refactor TimeTableX into modular C++17 components**. In the description, record your Windows test and demo results. Review **Files changed** and the CI result. Merge into `main` after the Windows checks, review, and CI pass. Use the merge-commit option if you want to retain the staged patch commits in the main branch history and your repository settings allow it.
