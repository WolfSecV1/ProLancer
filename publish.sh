#!/bin/bash

# 1. Get version from file (or hardcode it here)
VERSION=$(cat version.txt)

# 2. Add all changes
git add .

# 3. Commit with message
git commit -m "Release v$VERSION"

# 4. Push to main branch
git push origin main

# 5. Create git tag
git tag -a "v$VERSION" -m "Version $VERSION"
git push origin "v$VERSION"

# Optional: build step
# qmake && make
