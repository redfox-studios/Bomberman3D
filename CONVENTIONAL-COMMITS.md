# Conventional Commits - Quick Guide

Format: `type(scope): message`

Example: `feat(auth): add Google login`

## Types & When to Use

| Type       | When                                                                |
|------------|---------------------------------------------------------------------|
| `feat`     | New feature                                                         |
| `fix`      | Bug fix                                                             |
| `refactor` | Code change that's neither a fix nor feature (cleanup, restructure) |
| `style`    | Formatting only - no logic change (spaces, semicolons, etc.)        |
| `chore`    | Boring maintenance - updating deps, config, build scripts           |
| `docs`     | Documentation only                                                  |
| `test`     | Adding or fixing tests                                              |
| `perf`     | Performance improvement                                             |
| `revert`   | Reverting a previous commit                                         |

## Rules

- **Lowercase** everything
- **Present tense** - "add" not "added"
- **Short message** - under 72 chars
- Scope is optional but useful: `fix(cart): wrong price calculation`

## Breaking Changes

Add `!` before the colon: `feat(api)!: change response format`

## Examples

```
feat(user): add profile picture upload
fix(login): fix crash on wrong password
chore: update dependencies
refactor(db): simplify query logic
docs: update README setup steps
```

**If you're unsure** - ask yourself: *did I add something new?* → `feat`. *Did I break something and fix it?* → `fix`. *Everything else?* → pick the closest one from the table.