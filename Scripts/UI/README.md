# UI Subsystem - Accessibility Fixes and Mouse Management

My Contribution: Fixed bugs related to UI accessibility settings, with a primary focus on mouse behavior and cursor positioning during menu navigation.
---
## Key contributions include:

- Mouse Position Preservation: Implemented functionality to save the cursor's screen coordinates before opening a menu.
- Mouse Position Restoration: Restored the cursor to its previous position after the menu is opened, preventing unwanted cursor repositioning.
- UI Input Handling: Configured menu transitions to enable mouse cursor visibility and switch the PlayerController to UI-only input mode.
- Accessibility Bug Fixes: Investigated and resolved bugs affecting accessibility settings and the overall UI interaction experience.

The mouse-position functionality is handled through SaveMousePosition() and RestoreMousePosition(), ensuring a more consistent experience when transitioning between gameplay and menu interfaces.
