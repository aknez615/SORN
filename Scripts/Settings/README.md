# Player Settings and Accessibility System

My Contribution: Worked on implementing and troubleshooting player settings and accessibility features, including camera controls, visual adjustments, audio levels, and subtitle preferences. 

My primary focus was improving the accessibility settings system and ensuring player-configured options were correctly applied and maintained throughout gameplay.

---
## Key contributions include:

- Field of View (FOV): Implemented adjustable FOV settings, allowing players to customize their camera's viewing angle.
- Brightness Controls: Worked on brightness adjustments using a dynamic post-processing material to apply visual changes in-game.
- Mouse and Controller Sensitivity: Supported configurable mouse and controller sensitivity, including controller deadzones, response curves, and inverted Y-axis settings.
- Audio Controls: Implemented adjustable master, music, sound effects, and voice volume levels using Unreal Engine's Sound Mix system.
- Subtitle Controls: Integrated subtitle enable/disable settings with Unreal Engine's subtitle system.
- Camera Shake: Added the ability to enable/disable camera shake effects.
- Text Size: Supported multiple text-size options, including Small, Regular, Large, and Extra Large.
- Settings Persistence: Worked on saving and loading player preferences through Unreal Engine's UGameUserSettings, ensuring configurations persist between sessions.

These features are managed through USORNGameSettings and USettingsManagerSubsystems, which handle player preferences, settings persistence, and applying configured values to the game.
