# UI Button System Notes

## Overview

All buttons in the UI use a reusable widget:

WBP_Button

This keeps styling consistent across the whole game.

Instead of editing 50 buttons individually, styles are controlled from a single theme asset.

---

# Button Variants

Each button has a Variant property.

Available variants:

* Primary
* Secondary
* Destructive

Designer can set this per button in the Details panel.

Example:

Play → Primary
Settings → Secondary
Delete Save → Destructive

---

# Where Styles Are Stored

All button styles are stored in the UI theme data asset:

DA_UIColors

This asset contains:

PrimaryButtonStyle
SecondaryButtonStyle
DestructiveButtonStyle

Each style controls:

* Normal appearance
* Hover appearance
* Pressed appearance
* Disabled appearance
* Outline
* Sounds
* Padding
* Background brushes

Editing these styles will update every button that uses them.

---

# How Buttons Apply Styles

Inside WBP_Button the following logic happens:

Variant → Switch → Apply Style

Primary → PrimaryButtonStyle
Secondary → SecondaryButtonStyle
Destructive → DestructiveButtonStyle

The button reads the style from the UI theme asset and applies it automatically.

---

# How To Create A Button

1. Drag **WBP_Button** into the UI
2. Set the following properties:

ButtonText → text shown on button
Variant → Primary / Secondary / Destructive
UIColors → DA_UIColors

Example setup:

Play Button
ButtonText = "Play"
Variant = Primary

Settings Button
ButtonText = "Settings"
Variant = Secondary

Delete Button
ButtonText = "Delete Save"
Variant = Destructive

---

# When To Use Each Variant

Primary
Main actions the player should notice.

Secondary
Regular actions.

Destructive
Dangerous actions like deleting saves or resetting progress.

---

# Important

Do NOT change button colors directly in widgets.

Always edit styles in:

DA_UIColors

This ensures the entire UI stays consistent.
