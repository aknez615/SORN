# PlayerController

My Contribution: Implemented and configured player input sensitivity settings within the ASORNPlayerController. I worked on the input sensitivity system and adjusted it based on the needs of the project. This allowed players to customize their aiming and camera control experience through adjustable mouse and controller settings.

---
## Key features I worked on include:

- Mouse Sensitivity: Applied the player's configured mouse sensitivity to horizontal and vertical input.
- Controller Sensitivity: Applied configurable controller sensitivity to gamepad input.
- Controller Deadzone: Implemented deadzone processing to prevent unintended input from minor analog stick movement.
- Response Curve: Applied a configurable response curve to controller input for more customizable aiming behavior.
- Invert Y-Axis: Supported inverted vertical camera input through the player's settings.
- Input Normalization: Processed input values before applying sensitivity adjustments to maintain consistent control behavior.

These settings are processed through the ApplySensitivity() function and retrieved from USORNGameSettings, keeping input behavior connected to the game's player-configurable settings.
