/*
 * liteRadar is a simple library for using the Seeed 24GHz Human Static Presence Lite module
 * in straight forward motion and presence detection sensors.
 * 
 * The module may be used with preset scenarios and sensitivity settings or it may bs used in 
 * "custom" modes where presence and motion range and thresholds may be set. 
 * 
 */


#include "liteRadar.h"

Radar::Radar(Stream *s)
	: stream(s) {
	
}

/*!
 * @fn getFrame
 * @brief reads a frame from the radar module
 * @param frame frame structure to hold returned data and length read
 * @returns true is successful read, false if nothing to read
 */
bool Radar::getFrame(Frame* frame) {
	int l = 0;
	byte c;
	while (stream->available()) {
		c = stream->read();
		if (c == HEAD1) {
			frame->msg[l] = c;
			l++;
			c = stream->read();
			if (c == HEAD2) {
				frame->msg[l] = c;
				l++;
				c = stream->read();
				while (c != END2) {
					frame->msg[l] = c;
					l++; 
					c = stream->read();
				}
				frame->l = l+1;
				frame->msg[l] = c;
				return true;
			}
		}
	}
	frame->l = 0;
	return false;
}

/*!
 * @fn printFrame
 * @brief prints a frame to Serial. Primarily for debugging
 * @param frame frame structure to be printed
 */

void Radar::printFrame(Frame* frame) {
	char output[4];
	for (int n = 0; n < frame->l; n++) {
		sprintf(output, "%02X", frame->msg[n]);
		Serial.print(output);
		Serial.print(' ');
	}
	Serial.println();
	
}

/*!
 * @fn putFrame
 * @brief sends a frame to the radar module
 * @param frame frame structure to be sent
 */
void Radar::putFrame(Frame* frame) {
	stream->write(frame->msg, frame->l);
	stream->flush();
}

/*!
 * @fn calculateChecksum
 * @brief calculates the checksum and puts it into the frame
 * @param frame frame structure to be modified
 */
void Radar::calculateChecksum(Frame* frame) {

	byte checksum = 0;
	int data_byte = frame->l - 4;
	int cs_byte = frame->l - 3;
	for (int i = 0; i < cs_byte; i++) {
		checksum = checksum + frame->msg[i];
	}
	frame->msg[cs_byte] = checksum;
}

/*!
 * @fn setParam
 * @brief constructs a frame and sends it to the module. Then reads up to 10 following
 * 		frames to see if the correct response is received
 * @param control byte to hold control value
 * @param command byte to hold command specifiying parameter
 * @param value	byte to hold new value for parameter
 */
bool Radar::setParam(byte control, byte command, byte value) {									 // currently only works with single byte data
	Frame req {
	.msg = {HEAD1, HEAD2, control, command, 0x00, 0x01, 0x00, 0x00, END1, END2},
	.l = 10
	};

	Frame ret {
		.msg = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
	};

	unsigned long start, elapsed;

	req.msg[DATA] = value;
	calculateChecksum(&req);

	start = millis();

	putFrame(&req);
	while (true) {
		if (getFrame(&ret)) {
			if (ret.msg[CONTROL] == control) {
				if (ret.msg[COMMAND] == command || ret.msg[COMMAND] == INIT_COMPLETE) {
					elapsed = millis() - start;
					return true;
				}
			}
			elapsed = millis() - start;
			if (elapsed >= 2000) {
				return false;
			}
		}
	}

}

/*!
 * @fn resetRadar
 * @brief resets the radar module 
 * 		note that this does not factory reset all of the settings.
 * @returns
 */
bool Radar::resetRadar() {
	return setParam(SYSTEM, RESET, ZERO_F);
}

/*!
 * @fn setSenario
 * @brief sets the scenario used by the module
 * 		note that this does not factory reset all of the settings.
 * @param scenario  the scenario to be used 
 * 					can be LIVING_ROOM, AREA_DETECTION, BEDROOM, or BATHROOM
 * @returns true on success, false if failed
 */
bool Radar::setScenario(byte scenario) {
	return setParam(WORKING_STATUS, SET_SCENARIO, scenario);
}

/*!
 * @fn setSensitvity
 * @brief sets the sensitivity used by the module
 * 		can be 1-3
 * @returns true on success, false if failed
 */
bool Radar::setSensitivity(byte scenario) {
	return setParam(WORKING_STATUS, SET_SENSITIVITY, scenario);
}

/*!
 * @fn openCustomMode
 * @brief opens custom mode to allow more control of module settings
 * @param mode byte value 1-4
 * @returns true if success, false if failed
 */
bool Radar::openCustomMode(byte mode) {
	return setParam(WORKING_STATUS, OPEN_CUSTOM, mode);
}

/*!
 * @fn closeCustomMode
 * @brief closes custom mode and saves values to module
 * @returns true if success, false if failed
 */
bool Radar::exitCustomMode() {
	return setParam(WORKING_STATUS, EXIT_CUSTOM, ZERO_F);
}

/*!
 * @fn setPresenceThreshold
 * @brief set presence threshold for the current custom mode
 * @param byte value between 0 and 250
 * @returns true for success, false for failed
 */
bool Radar::setPresenceThreshold(byte threshold) {
	return setParam(CUSTOM, SET_PRESENCE_THRESHOLD, threshold);
}

/*!
 * @fn setPresenceRange
 * @brief set presence range for the current custom mode
 * @param byte values from 0 (0m) to 0A (5m) are valid
 * @returns true for success, false for failed
 */
bool Radar::setPresenceRange(byte range) {
	return setParam(CUSTOM, SET_PRESENCE_RANGE, range);
}

/*!
 * @fn setTimeOfAbsence
 * @brief set the time to wait before absence is reported
 * @param byte value between 0 and 08
 * @returns true for success, false for failed
 */
bool Radar::setTimeOfAbsence(byte t) {
	return setParam(WORKING_STATUS, SET_TIME_OF_ABSENCE, t);
}

/*!
 * @fn setMotionThreshold
 * @brief set motion threshold for the current custom mode
 * @param byte value between 0 and 250
 * @returns true for success, false for failed
 */
bool Radar::setMotionThreshold(byte threshold){
	return setParam(CUSTOM, SET_MOTION_THRESHOLD, threshold);
}

/*!
 * @fn setMotionRange
 * @brief set motion range for the current custom mode
 * @param byte values from 0 (0m) to 0A (5m) are valid
 * @returns true for success, false for failed
 */
bool Radar::setMotionRange(byte range) {
	return setParam(CUSTOM, SET_MOTION_RANGE, range);
}

/*!
 * @fn isPresent
 * @brief returns current state of presence
 * @returns true for presnce, false for absence
 */
bool Radar::isPresent() {
	return presence;
}

/*!
 * @fn isMoving
 * @brief returns current state of motion
 * @returns true for motion, false for no motion
 */
bool Radar::isMoving() {
	if (motion == 0x02) return true;
	return false;
}

/*!
 * @fn updateStatus
 * @brief goes in loop to  get frames abd update presnec snd motion status. It is non blocking and
 * passes through if no frames are available.
 * @returns true for new data, false for no change
 */

bool Radar::updateStatus() {
	Frame f;
	bool changed = false;
	if (getFrame(&f)) {
		switch (f.msg[CONTROL]) {
			case HUMAN_STATUS:
				switch (f.msg[COMMAND]) {
					case PRESENCE:
						if (f.msg[DATA] != presence) {
							presence = f.msg[DATA];
							changed = true;
						}
						break;
					case MOTION:
						if (f.msg[DATA] != motion) {
							motion = f.msg[DATA];
							changed = true;
						}
						break;
					default:
						changed = false;
						break;

				}
		}
	return changed;
	}
}