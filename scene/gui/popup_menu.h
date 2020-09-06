/*************************************************************************/
/*  popup_menu.h                                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef POPUP_MENU_H
#define POPUP_MENU_H

#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/popup.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/split_container.h"
#include "scene/gui/shortcut.h"
#include "scene/gui/button.h"
#include "scene/gui/check_button.h"
#include "scene/gui/check_box.h"

class PopupMenu : public Popup {
	GDCLASS(PopupMenu, Popup);

	Timer *submenu_timer;
	List<Rect2> autohide_areas;
	Vector<Control *> items;
	int initial_button_mask;
	bool during_grabbed_click;
	int mouse_over;
	int submenu_over;
	Rect2 parent_rect;
	String _get_accel_text(int p_item) const;
	int _get_mouse_over(const Point2 &p_over) const;
	virtual Size2 _get_contents_minimum_size() const override;

	int _get_items_total_height() const;
	void _scroll_to_item(int p_item);

	void _gui_input(const Ref<InputEvent> &p_event);
	void _activate_submenu(int over);
	void _submenu_timeout();

	uint64_t popup_time_msec = 0;
	bool hide_on_item_selection;
	bool hide_on_checkable_item_selection;
	bool hide_on_multistate_item_selection;
	Vector2 moved;

	Array _get_items() const;
	void _set_items(const Array &p_items);

	Map<Ref<Shortcut>, int> shortcut_refcount;

	void _ref_shortcut(Ref<Shortcut> p_sc);
	void _unref_shortcut(Ref<Shortcut> p_sc);

	bool allow_search;
	uint64_t search_time_msec;
	String search_string;

	MarginContainer *margin_container;
	ScrollContainer *scroll_container;
	VBoxContainer *item_container;

	void _draw_items();
	void _draw_background();

protected:
	friend class MenuButton;
	void _notification(int p_what);
	static void _bind_methods();

private:
	void _select_item(Control *p_item);

public:
	void add_item(Control *p_item);

	Button *add_button(const String &p_label, Ref<ShortCut> p_shortcut = Ref<ShortCut>(), Ref<Texture2D> p_icon = Ref<Texture2D>());
	CheckBox *add_check_button(const String &p_label, Ref<ShortCut> p_shortcut = Ref<ShortCut>(), Ref<Texture2D> p_icon = Ref<Texture2D>());
	CheckBox *add_radio_button(const String &p_label, Ref<ButtonGroup> p_group, Ref<ShortCut> p_shortcut = Ref<ShortCut>(), Ref<Texture2D> p_icon = Ref<Texture2D>());

	// Helper method since buttons with icons but without shortcuts are common.
	// TODO: Question: Should every button have an icon and a shortcut (Configurable in editor theme and settings, respectively)
	//       If so, we should remove this method in favor of "add_button"
	Button *add_icon_button(const String &p_label, Ref<Texture2D> p_icon, Ref<ShortCut> p_shortcut = Ref<ShortCut>());

	Label *add_label(const String &p_label);

	SplitContainer *add_separator();

	Control *get_item(const int &p_id);

	Button *get_button(const int &p_id);
	CheckBox *get_check_button(const int &p_id);
	CheckBox *get_radio_button(const int &p_id);

	int get_current_index() const;
	int get_item_count() const;

	void remove_item(const int &p_id);

	void clear();

	void set_parent_rect(const Rect2 &p_rect);

	virtual String get_tooltip(const Point2 &p_pos) const;

	virtual void get_translatable_strings(List<String> *p_strings) const override;

	void add_autohide_area(const Rect2 &p_area);
	void clear_autohide_areas();

	void set_hide_on_item_selection(bool p_enabled);
	bool is_hide_on_item_selection() const;

	void set_hide_on_checkable_item_selection(bool p_enabled);
	bool is_hide_on_checkable_item_selection() const;

	void set_hide_on_multistate_item_selection(bool p_enabled);
	bool is_hide_on_multistate_item_selection() const;

	void set_submenu_popup_delay(float p_time);
	float get_submenu_popup_delay() const;

	void set_allow_search(bool p_allow);
	bool get_allow_search() const;

	virtual void popup(const Rect2 &p_bounds = Rect2());

	void take_mouse_focus();

	PopupMenu();
	~PopupMenu();
};

#endif
