# OpenGL-Racing-Simulator
3D Racing Simulator, programmed in C++ using OpenGL. Features a single racetrack with curves, straights, sharp turns, and a lap timing system. The objective is to achieve the shortest lap time while navigating through the track.

<img width="1068" alt="Screenshot 2024-06-18 at 12 23 51 PM" src="https://github.com/Joshuliu/OpenGL-Racing-Simulator/assets/13361132/f8ae05f7-1281-464f-889d-c18b932d6765">

## Features

- **Realistic Racecar Physics**: Acceleration, braking, and steering mechanics closely mimic real-world physics.
- **Dynamic Lighting**: Includes day/night cycles with headlight effects to enhance visual realism.
- **Lap Timing System**: Tracks your best lap times with checkpoint validations to ensure the entire track is completed.
- **Interactive Controls**: Support for keyboard inputs to control the car, including acceleration, braking, and steering.
- **Environment**: Detailed track environments with grass, trees, clouds, and audience stands.

## Installation

### Prerequisites

To run Racing Simulator, ensure you have the following installed:
- OpenGL
- GLUT (OpenGL Utility Toolkit)

## Controls
### General Controls:
	ESC - Exit the game.
	R - Reset the game state, repositioning the vehicle and resetting lap timing and velocity.
### Movement Controls:
	W - Accelerate the vehicle. Speed increases until it reaches a maximum limit.
	S - Decelerate or reverse the vehicle. Speed decreases, and the vehicle can move backward.
	A - Steer left. Adjusts the wheel angle to the left, up to a maximum angle.
	D - Steer right. Adjusts the wheel angle to the right, down to a minimum angle.
### Camera and View Controls:
	C - Toggle rear view (look behind).
	Q - Look left without changing the direction of movement.
	E - Look right without changing the direction of movement.
	X - Toggle between first-person view (FPV) and third-person view.
### Miscellaneous Controls:
	H - Cycle through different headlight modes: Auto, Off, Low, High.
### Stepping Controls (stepping through walls):
	Arrow Up - Move forward relative to the vehicle’s current direction.
	Arrow Down - Move backward relative to the vehicle’s current direction.
### Steering Controls:
	Arrow Left - Rotate the vehicle’s view left without moving.
	Arrow Right - Rotate the vehicle’s view right without moving.
### Mouse Controls:
	Left click start window to select options and start the game.
	Right click gameplay window to use popup menu and change day/night settings.
## Contributing
Contributions to the Racing Simulator are welcome. Please fork the repository and submit a pull request with your features or fixes.
