/*
 * Copyright (C) 2001-2025 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DCPLUSPLUS_WIN32_MDI_CHILD_FRAME_H_
#define DCPLUSPLUS_WIN32_MDI_CHILD_FRAME_H_

#include <string>

#include <dcpp/SettingsManager.h>
#include <dcpp/WindowInfo.h>

#include <dwt/widgets/MDIChild.h>
#include <dwt/util/StringUtils.h>

#include "AspectStatus.h"
#include "WinUtil.h"
#include "resource.h"

using std::string;

using dwt::util::escapeMenu;

class MDIChildFrameBase : public dwt::MDIChild
{
protected:
	explicit MDIChildFrameBase(MDIParentPtr parent) : dwt::MDIChild(parent), icon() { }
	virtual ~MDIChildFrameBase() { }

public:
	virtual const string& getId() const {
		return Util::emptyString;
	}

	virtual WindowParams getWindowParams() const {
		return WindowParams();
	}

	const dwt::IconPtr& getIcon() const {
		return icon;
	}

protected:
	void setWindowIcon(const dwt::IconPtr& icon_) {
		icon = icon_;
		setSmallIcon(icon);
		setLargeIcon(icon);
	}

private:
	dwt::IconPtr icon;
};

template<typename T>
class MDIChildFrame :
	public MDIChildFrameBase,
	public AspectStatus<T>
{
	typedef MDIChildFrameBase BaseType;
	typedef MDIChildFrame<T> ThisType;

	const T& t() const { return *static_cast<const T*>(this); }
	T& t() { return *static_cast<T*>(this); }

protected:
	MDIChildFrame(MDIParentPtr parent, const tstring& title, unsigned helpId = 0, unsigned iconId = 0, bool manageAccels = true) :
		BaseType(parent),
		lastFocus(NULL),
		alwaysSameFocus(false),
		dirty(false),
		closing(false)
	{
		dwt::MDIChild::Seed cs(title);
		cs.activate = false;
		this->createMDIChild(cs);

		if(helpId)
			setHelpId(helpId);

		if(iconId)
			setIcon(iconId);

		this->onContextMenu([this](const dwt::ScreenCoordinate &sc) { return this->handleContextMenu(sc); });

		onClosing([this] { return this->handleClosing(); });
		onFocus([this] { this->handleFocus(); });
		onWindowPosChanged([this](const dwt::Rectangle &r) { this->handleSized(r); });
		this->addCallback(dwt::Message(WM_MDIACTIVATE), [this](const MSG& msg, LRESULT&) -> bool {
			this->handleActivation(reinterpret_cast<HWND>(msg.lParam) == this->handle());
			return false;
		});
		addDlgCodeMessage(this);

		addAccel(FCONTROL, 'W', [this] { this->close(true); });
		if(manageAccels)
			initAccels();
	}

	virtual ~MDIChildFrame() { }

	/**
	 * The first of two close phases, used to disconnect from other threads that might affect this window.
	 * This is where all stuff that might be affected by other threads goes - it should make sure
	 * that no more messages can arrive from other threads
	 * @return True if close should be allowed, false otherwise
	 */
	bool preClosing() { return true; }
	/** Second close phase, perform any cleanup that depends only on the UI thread */
	void postClosing() { }

	enum FocusMethod { AUTO_FOCUS, ALWAYS_FOCUS, NO_FOCUS };

	template<typename W>
	void addWidget(W* widget, FocusMethod focus = AUTO_FOCUS, bool autoTab = true, bool customColor = true) {
		addDlgCodeMessage(widget, autoTab);

		if(customColor)
			addColor(widget);

		if(focus == ALWAYS_FOCUS || (focus == AUTO_FOCUS && !lastFocus)) {
			lastFocus = widget->handle();
			if(this->getVisible())
				::SetFocus(lastFocus);
		}
		if(focus == ALWAYS_FOCUS)
			alwaysSameFocus = true;
	}

	void setDirty(SettingsManager::BoolSetting setting) {
		if(SettingsManager::getInstance()->get(setting) && !isActive()) {
			dirty = true;
			::FlashWindow(handle(), TRUE);
		}
	}

	void activate() {
		BaseType::activate();
	}

	bool isActive() const {
		return getParent()->getActive() == this;
	}

	MDIParentPtr getParent() const {
		return const_cast<MDIParentPtr>(BaseType::getParent());
	}

	void setIcon(dwt::IconPtr icon) {
		setWindowIcon(icon);
	}
	void setIcon(unsigned iconId) {
		setIcon(WinUtil::tabIcon(iconId));
	}

	static bool parseActivateParam(const WindowParams& params) {
		auto i = params.find("Active");
		return i != params.end() && i->second == "1";
	}

private:
	HWND lastFocus; // last focused widget
	bool alwaysSameFocus; // always focus the same widget
	bool dirty;

	bool closing;

	void addDlgCodeMessage(ComboBox* widget, bool autoTab = true) {
		widget->onRaw([=](WPARAM w, LPARAM) { return this->handleGetDlgCode(w, autoTab); }, dwt::Message(WM_GETDLGCODE));
		auto text = widget->getTextBox();
		if(text)
			text->onRaw([=](WPARAM w, LPARAM) { return this->handleGetDlgCode(w, autoTab); }, dwt::Message(WM_GETDLGCODE));
	}

	template<typename W>
	void addDlgCodeMessage(W* widget, bool autoTab = true) {
		widget->onRaw([=](WPARAM w, LPARAM) { return this->handleGetDlgCode(w, autoTab); }, dwt::Message(WM_GETDLGCODE));
	}

	void addColor(ComboBox* widget) {
		// do not apply our custom colors to the combo itself, but apply it to its drop-down and edit controls

		auto listBox = widget->getListBox();
		if(listBox)
			addColor(listBox);

		auto text = widget->getTextBox();
		if(text)
			addColor(text);
	}

	// do not apply our custom colors to Buttons and Button-derived controls
	void addColor(dwt::Button* widget) {
		// empty on purpose
	}

	template<typename A>
	void addColor(dwt::aspects::Colorable<A>* widget) {
		WinUtil::setColor(static_cast<dwt::Control*>(widget));
	}

	// Catch-rest for the above
	void addColor(void* w) {

	}

	void handleSized(const dwt::Rectangle&) {
		t().layout();
	}

	void handleFocus() {
		if(lastFocus) {
			::SetFocus(lastFocus);
		}
	}

	void handleActivation(bool active) {
		if(active) {
			if(dirty) {
				dirty = false;
				::FlashWindow(handle(), FALSE);
			}
			handleFocus();
		} else if(!alwaysSameFocus) {
			// remember the previously focused window.
			HWND focus = ::GetFocus();
			if(focus && ::IsChild(t().handle(), focus))
				lastFocus = focus;
		}
	}

	LRESULT handleGetDlgCode(WPARAM wParam, bool autoTab) {
		if(autoTab && wParam == VK_TAB)
			return 0;
		return DLGC_WANTMESSAGE;
	}

	bool handleContextMenu(const dwt::ScreenCoordinate& pt) {
		auto menu = addChild(WinUtil::Seeds::menu);

		menu->setTitle(escapeMenu(getText()), getIcon());

		windowMenuImpl(menu.get());
		menu->appendItem(T_("&Close"), [this] { close(true); }, WinUtil::menuIcon(IDI_EXIT));

		menu->open(pt);
		return true;
	}

	bool handleClosing() {
		if(!closing && t().preClosing()) {
			closing = true;
			// async to make sure all other async calls have been consumed
			this->callAsync([this] {
				this->t().postClosing();
				this->getParent()->destroy(this);
			});
			return false;
		}
		return false;
	}

	virtual void windowMenuImpl(dwt::Menu* menu) {
		// empty on purpose; implement this in the derived class to modify the window menu.
	}
};

#endif
