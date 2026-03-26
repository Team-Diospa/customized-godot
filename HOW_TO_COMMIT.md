# Team Guide: How to Commit

This document outlines the version control rules for the Game Repository. Refer to this when committing changes to the game project.

## Rule 1: No Direct Pushes to Main
The main branch must remain stable. 
1. Create a new branch for your task (e.g., level/dark-forest, art/hero-model).
2. Commit changes to your branch.
3. Open a Pull Request for review before merging.

## Rule 2: Git LFS (Large File Storage)
Git LFS handles large binary files to prevent repository bloating.
- .gitattributes is configured to track .png, .wav, .blend, .fbx, .exr, and .mp4 files.
- Run `git lfs pull` after your initial clone to download the asset files.

## Rule 3: Commit Messages
Commit messages must explain what was changed and the reason for the change.
- Incorrect: "Fixed stuff"
- Correct: "Added pine tree variants to the Dark Forest chunk 1A"
- Correct: "Rebalanced sword base damage from 25 to 35"

## Daily Workflow
1. Run `git checkout main` and `git pull`.
2. Run `git checkout -b your-branch-name`.
3. Make your changes locally.
4. Run `git add .` to stage files.
5. Run `git commit -m "Descriptive message"`.
6. Run `git push -u origin your-branch-name`.
7. Open a Pull Request on the repository host.
