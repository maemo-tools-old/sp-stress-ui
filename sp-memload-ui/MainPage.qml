/*
 * Copyright (C) 2011 Nokia Corporation.
 *
 * Author: Tommi Rantala <ext-tommi.1.rantala@nokia.com>
 * Contact: Eero Tamminen <eero.tamminen@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

import QtQuick 1.1
import com.nokia.meego 1.0
import com.nokia.endurance 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Page {
	id: mainpage
	anchors.bottomMargin: UI.DEFAULT_MARGIN

	MemoryLoad {
		id: memload
		onBusyChanged: cgroupInfo.update()
	}
	CgroupInfo {
		id: cgroupInfo
	}

	function update() {
		memload.updateAllocationInfo();
		cgroupInfo.update();
	}

	/* Policy (eventually) changes our control group after swiping the
	 * application to background. Do one-shot updates so that we might show
	 * the new group in the thumbnail.
	 */
	Timer {
		id: delayedStatisticsUpdate100ms
		interval: 100
		repeat: false
		onTriggered: update();
	}
	Timer {
		id: delayedStatisticsUpdate2sec
		interval: 2000
		repeat: false
		onTriggered: update();
	}

	Connections {
		target: platformWindow
		onActiveChanged: {
			update();
			delayedStatisticsUpdate100ms.restart()
			delayedStatisticsUpdate2sec.restart()
		}
	}
	Rectangle {
		id: header
		x: 0
		y: 0
		width: parent.width
		height: UI.HEADER_DEFAULT_HEIGHT_PORTRAIT
		clip: true
		color: "#E00000"
		Text {
			id: progname
			text: "sp-memload"
			font.pixelSize: UI.FONT_XLARGE
			font.family: UI.FONT_FAMILY_LIGHT
			color: UI.COLOR_INVERTED_FOREGROUND
			x: UI.DEFAULT_MARGIN
			y: UI.HEADER_DEFAULT_TOP_SPACING_PORTRAIT
		}
		BusyIndicator {
			opacity: memload.busy
			running: memload.busy
			x: parent.width - width - UI.DEFAULT_MARGIN
			y: UI.HEADER_DEFAULT_TOP_SPACING_PORTRAIT
			Behavior on opacity { NumberAnimation { } }
			platformStyle: BusyIndicatorStyle { inverted: true }
		}
		z: 10
	}
	Flickable {
		width: parent.width - 2*UI.DEFAULT_MARGIN
		x: UI.DEFAULT_MARGIN
		anchors.topMargin: UI.DEFAULT_MARGIN
		anchors.top: header.bottom
		anchors.bottom: parent.bottom
		contentHeight: content.height
		flickableDirection: Flickable.VerticalFlick
		Column {
			id: content
			width: parent.width
			spacing: 6
			Label {
				text: qsTr("Memory Allocation")
				width: parent.width
				font.pixelSize: UI.FONT_SLARGE
				font.bold: true
			}
			Row {
				width: parent.width
				enabled: !memload.busy
				Label {
					width: parent.width / 4
					text: qsTr("Size:")
					verticalAlignment: Text.AlignVCenter
					height: newAllocation.height
					font.bold: true
				}
				TextField {
					id: newAllocation
					placeholderText: qsTr("Megabytes")
					width: 3 * parent.width / 4
					onAccepted: memload.setAllocatedMemory(text);
					validator: IntValidator { bottom: 1 }
				}
			}
			Row {
				width: parent.width
				enabled: !memload.busy
				spacing: 4
				Label {
					width: parent.width / 4 - 2
					text: qsTr("Fill:")
					verticalAlignment: Text.AlignVCenter
					height: fillStyle.height
					font.bold: true
				}
				ButtonRow {
					id: fillStyle
					width: 3 * parent.width / 4 - 2
					checkedButton: memload.randomizedFill
						? randomizedFillButton : zeroFillButton
					Button {
						id: zeroFillButton
						text: qsTr("0x00")
						onClicked: {
							memload.setRandomizedFill(false);
							memload.setFillPattern(0);
						}
					}
					Button {
						id: randomizedFillButton
						text: qsTr("Randomized")
						onClicked: memload.setRandomizedFill(true);
					}
				}
			}
			Item { width: parent.width; height: 4 }
			Row {
				id: readWriteButtons
				width: parent.width
				spacing: UI.BUTTON_SPACING
				enabled: !memload.busy && memload.allocatedMemory > 0
				Button {
					id: rereadButton
					width: parent.width / 2 - 2
					text: qsTr("Re-read")
					onClicked: memload.memoryRead();
				}
				Button {
					width: parent.width / 2 - 2
					text: qsTr("Rewrite")
					onClicked: memload.memoryWrite();
				}
			}
			Row {
				width: parent.width
				spacing: UI.BUTTON_SPACING
				Button {
					text: qsTr("Free")
					onClicked: memload.freeAllocation();
					width: parent.width / 2 - 2
					enabled: !memload.busy && memload.allocatedMemory > 0
				}
				Button {
					text: qsTr("Allocate")
					onClicked: memload.setAllocatedMemory(newAllocation.text);
					width: parent.width / 2 - 2
					enabled: !memload.busy
				}
			}
			Label {
				text: qsTr("Allocation failed!")
				color: "red"
				visible: memload.allocationFailed
				width: parent.width
				horizontalAlignment: Text.AlignHCenter
			}
			Label {
				id: allocationLabel
				width: parent.width
				text: {
					if (memload.allocatedMemory > 0) {
						return qsTr("Allocation size is <b>%1&nbsp;MB</b> or <b>%2&nbsp;kB</b>. " +
							"Statistics for the virtual memory area that contains the allocation:" +
							"<pre>%3</pre>")
							.arg(memload.allocatedMemory)
							.arg(memload.allocatedMemory * 1024)
							.arg(memload.allocationInfo)
					} else {
						return "";
					}
				}
				height: text == "" ? 0 : implicitHeight
				clip: true
				Behavior on height { NumberAnimation { easing.type: Easing.OutQuad } }
			}
			Button {
				id: updateStatisticsButton
				width: parent.width
				text: qsTr("Update statistics")
				opacity: memload.allocatedMemory > 0
				onClicked: memload.updateAllocationInfo();
			}

			Item {
				width: parent.width
				height: 16
			}
			Label {
				text: qsTr("Control Group")
				font.pixelSize: UI.FONT_SLARGE
				font.bold: true
				width: parent.width
			}
			Label {
				width: parent.width
				text: {
					var cg = cgroupInfo.cgroup.split("/");
					var result = "";
					var indent = 0;
					for (var c in cg) {
						if (cg[c] == "")
							continue;
						if (indent == 0) {
							result += ("/" + cg[c]);
						} else {
							var i = "<br/>";
							for (var j = 0; j < indent ; ++j)
								i += "&nbsp;&nbsp;&nbsp;";
							result += (i + "\u2192/" + cg[c]);
						}
						++indent;
					}
					return result;
				}
			}
			Label {
				text: qsTr("Group statistics")
				font.bold: true
				width: parent.width
			}
			Label {
				width: parent.width
				visible: cgroupInfo.memoruUsage != -1
				text: qsTr("Memory usage: <b>%1&nbsp;MB</b> or <b>%2&nbsp;kB</b>.")
					.arg(Math.floor(cgroupInfo.memoryUsage / 1024 / 1024))
					.arg(cgroupInfo.memoryUsage / 1024)
			}
			Label {
				width: parent.width
				visible: cgroupInfo.memoruUsage != -1 && cgroupInfo.memoryLimit != -1
				text: qsTr("Memory limit: <b>%1&nbsp;MB</b> or <b>%2&nbsp;kB</b> (%3% used).")
					.arg(Math.floor(cgroupInfo.memoryLimit / 1024 / 1024))
					.arg(cgroupInfo.memoryLimit / 1024)
					.arg(Math.floor(100 * (cgroupInfo.memoryUsage / cgroupInfo.memoryLimit)))
			}
			Label {
				width: parent.width
				visible: cgroupInfo.swapUsage != -1
				text: qsTr("Swap usage: <b>%1&nbsp;MB</b>.")
					.arg(Math.floor(cgroupInfo.swapUsage / 1024 / 1024))
			}
			Button {
				width: parent.width
				text: qsTr("Update")
				onClicked: cgroupInfo.update();
			}
		}
	}
}
