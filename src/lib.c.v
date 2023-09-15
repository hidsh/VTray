module vtray

import builtin.wchar

#flag -I @VMODROOT/c/vtray.h

$if windows {
	#flag @VMODROOT/c/windows/tray.c
	#include "@VMODROOT/c/windows/tray.h"
} $else $if linux {
	#flag -I /usr/include/libappindicator3-0.1/libappindicator
	#flag @VMODROOT/c/linux/tray.c
	#include "@VMODROOT/c/linux/tray.h"
}

$if linux {
	#pkgconfig gtk+-3.0
}

struct C.VTray {}

pub type Vtray = C.VTray

fn C.vtray_init_windows(identifier &char, icon &char, tooltip &wchar.Character) &Vtray
fn C.vtray_run_windows(tray &Vtray)
fn C.vtray_exit_windows(tray &Vtray)
fn C.vtray_init_linux(identifier &char, icon &char, tooltip &char) &Vtray
fn C.vtray_run_linux(tray &Vtray)
fn C.vtray_exit_linux(tray &Vtray)
