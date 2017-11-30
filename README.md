# OSVR SteamVR Bridge
A driver for SteamVR that adds support for OSVR defined tracked controllers and HMDs

# How to use:
This driver looks for a running OSVR Server and tries to connect to the following paths:
```json
"aliases": {
		"/controller1/pose": 			[tracker],
		"/controller1/trigger": 		[analog 0-255],
		"/controller1/touch_x":			[analog 0-255],
		"/controller1/touch_y":			[analog 0-255],
		"/controller1/touch_touched":	[button],
		"/controller1/touch_pressed":	[button],
		"/controller1/system_pressed":	[button],
		"/controller1/grip_pressed":	[button],
		"/controller1/menu_pressed":	[button],
		
		"/controller2/pose": 			[tracker],
		"/controller2/trigger": 		[analog 0-255],
		"/controller2/touch_x":			[analog 0-255],
		"/controller2/touch_y":			[analog 0-255],
		"/controller2/touch_touched":	[button],
		"/controller2/touch_pressed":	[button],
		"/controller2/system_pressed":	[button],
		"/controller2/grip_pressed":	[button],
		"/controller2/menu_pressed":	[button],
		
		"/hmd/pose":					[tracker]
	}
```
It also creates a window on the monitor to the right of your primary monitor. It's set up for my headset currently and the values are hardcoded, but configuration support will be coming soon.