# UI Component System Notes

## Overview

UI use a reusable widgets:

- WBP_Button
- WBP_BlurryBackground
- WBP_GameTip
- WBP_RandomImage
- WBP_TransparentBackground

This keeps styling consistent across the whole game.

Instead of editing 50 buttons or other stuff individually, styles are controlled from a single theme asset.

---

# Button Variants

Each Component has a Variant property.

---

# Where Styles Are Stored

All component styles are stored in the UI theme data asset:

- DA_UIColors

There is a plenty of options for each component, if components dont have properties like colors, etc. These are handled in the component BP.

Editing these styles will update every button that uses them.

---

# How to Apply Styles

Example with WBP_Button

Inside WBP_Button the following logic happens:

Variant (Switch) -> Apply Style

Primary -> PrimaryButtonStyle
Secondary -> SecondaryButtonStyle
Destructive -> DestructiveButtonStyle

The button reads the style from the UI theme asset and applies it automatically.

---

# How To Create A Button

1. Add WBP_Button into the UI
2. Set the properties

---

# Important

Do NOT change colors directly in widgets or in BP_UIColors

Always edit styles in:

DA_UIColors

This ensures the entire UI stays consistent.
