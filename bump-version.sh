#!/bin/bash

# Usage: ./bump-version.sh [major|minor|patch] [version]

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

if ! git rev-parse --git-dir > /dev/null 2>&1; then
    print_error "Not in a git repository"
    exit 1
fi

if ! git diff-index --quiet HEAD --; then
    print_error "Working tree is not clean. Please commit or stash changes first."
    exit 1
fi

if [ ! -f ".VERSION" ]; then
    print_error "VERSION file not found"
    exit 1
fi

CURRENT_VERSION=$(cat .VERSION)
print_status "Current version: $CURRENT_VERSION"

IFS='.' read -r MAJOR MINOR PATCH <<< "$CURRENT_VERSION"

if [ $# -eq 0 ]; then
    echo "Usage: $0 [major|minor|patch|custom] [version]"
    echo "Current version: $CURRENT_VERSION"
    exit 1
fi

BUMP_TYPE=$1

case $BUMP_TYPE in
    "major")
        NEW_MAJOR=$((MAJOR + 1))
        NEW_MINOR=0
        NEW_PATCH=0
        NEW_VERSION="$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH"
        ;;
    "minor")
        NEW_MAJOR=$MAJOR
        NEW_MINOR=$((MINOR + 1))
        NEW_PATCH=0
        NEW_VERSION="$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH"
        ;;
    "patch")
        NEW_MAJOR=$MAJOR
        NEW_MINOR=$MINOR
        NEW_PATCH=$((PATCH + 1))
        NEW_VERSION="$NEW_MAJOR.$NEW_MINOR.$NEW_PATCH"
        ;;
    "custom")
        if [ $# -ne 2 ]; then
            print_error "Custom version requires a version argument"
            echo "Usage: $0 custom <version>"
            exit 1
        fi
        NEW_VERSION=$2
        if ! [[ $NEW_VERSION =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
            print_error "Invalid version format. Use semantic versioning (e.g., 1.2.3)"
            exit 1
        fi
        ;;
    *)
        print_error "Invalid bump type. Use: major, minor, patch, or custom"
        exit 1
        ;;
esac

print_status "Bumping version from $CURRENT_VERSION to $NEW_VERSION"

read -p "Continue? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    print_warning "Version bump cancelled"
    exit 0
fi

echo "$NEW_VERSION" > .VERSION
print_success "Updated .VERSION file"

if grep -q "v$CURRENT_VERSION" README.md; then
    sed -i "s/v$CURRENT_VERSION/v$NEW_VERSION/g" README.md
    print_success "Updated README.md"
fi

print_status "Building project with new version..."
make clean > /dev/null 2>&1
if make all > /dev/null 2>&1; then
    print_success "Build successful"
else
    print_error "Build failed! Rolling back changes..."
    echo "$CURRENT_VERSION" > .VERSION
    git checkout README.md 2>/dev/null || true
    exit 1
fi

print_status "Creating git commit and tag..."

git add .VERSION README.md

git commit -m "Bump version to $NEW_VERSION

Generated with bump-version.sh"

git tag -a "v$NEW_VERSION" -m "Release version $NEW_VERSION"

print_success "Created commit and tag v$NEW_VERSION"

print_status "Creating release archive..."
if make release > /dev/null 2>&1; then
    print_success "Release archive created: build/archium-$NEW_VERSION.tar.gz"
else
    print_warning "Release creation failed, but version bump was successful"
fi

print_success "Version bump complete!"
print_status "Next steps:"
echo "  1. Review the changes: git show"
echo "  2. Push changes: git push && git push --tags"
echo "  3. Create GitHub release with: build/archium-$NEW_VERSION.tar.gz"
