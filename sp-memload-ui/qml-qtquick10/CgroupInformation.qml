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
import com.nokia.SpStressUi 1.0
import "file:///usr/lib/qt4/imports/com/nokia/meego/UIConstants.js" as UI

Column {
	property CgroupInfo cgroupInfo
	visible: cgroupInfo.haveCgroup
	spacing: 6
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
	Column {
		visible: cgroupInfo.haveCgroupStatistics
		spacing: 6
		width: parent.width
		Label {
			text: qsTr("Group statistics")
			font.bold: true
			width: parent.width
		}
		Label {
			width: parent.width
			visible: cgroupInfo.memoryUsage != -1
			text: qsTr("Memory usage: <b>%1&nbsp;MB</b> or <b>%2&nbsp;kB</b>.")
				.arg(Math.floor(cgroupInfo.memoryUsage / 1024 / 1024))
				.arg(cgroupInfo.memoryUsage / 1024)
		}
		Label {
			width: parent.width
			visible: cgroupInfo.memoryUsage != -1 && cgroupInfo.memoryLimit != -1
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
	}
	Button {
		width: parent.width
		text: qsTr("Update")
		onClicked: cgroupInfo.update();
	}
}
