module vtray

// Tray is the main struct that represents the tray app.
[noinit]
pub struct Tray {
mut:
	instance   &VTray = unsafe { nil }
	icon       string
	identifier string
	tooltip    string
	items      []&MenuItem
	callbacks  map[int]fn ()
	last_id    int
}

[params]
pub struct CreatOptions {
	identifier string = 'VTray'
	tooltip    string
}

// MenuItem is a menu item that can be added to the tray.
pub struct MenuItem {
mut:
	id int
pub mut:
	text      string
	checked   bool
	checkable bool
	disabled  bool
	on_click  ?fn ()
}

// For MacOS the tray icon size must be 22x22 pixels in order for it to render correctly.
pub fn (mut t Tray) init() {
	t.instance = C.vtray_init(&VTrayParams{
		identifier: t.identifier
		tooltip: t.tooltip
		icon: t.icon
		on_click: fn [t] (menu_item &MenuItem) {
			if cb := t.callbacks[menu_item.id] {
				cb()
			}
		}
	}, usize(t.items.len), t.items.data)
}

// create Create a Tray.
pub fn create(icon_path string, opts CreatOptions) &Tray {
	return &Tray{
		icon: icon_path
		identifier: opts.identifier
		tooltip: opts.tooltip
	}
}

pub fn (mut t Tray) add_item(item MenuItem) {
	id := t.last_id++
	t.items << &MenuItem{
		...item
		id: id
	}
	if cb := item.on_click {
		t.callbacks[id] = cb
	}
}

// run Run the tray app.
pub fn (t &Tray) run() {
	C.vtray_run(t.instance)
}

// destroy Destroy the tray app and free the memory.
pub fn (t &Tray) destroy() {
	C.vtray_exit(t.instance)
}
