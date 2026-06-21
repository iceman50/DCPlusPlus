/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2013, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor the names of its contributors
        may be used to endorse or promote products derived from this software
        without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DWT_TASKBAR_H
#define DWT_TASKBAR_H

#include "WindowsHeaders.h"
#include "forward.h"
#include "Rectangle.h"
#include "resources/Bitmap.h"
#include "resources/Icon.h"
#include "tstring.h"
#include <unordered_map>
#include <vector>

namespace dwt {

struct JumpListLink {
	JumpListLink() :
		iconIndex(0),
		separator(false)
	{
	}

	tstring title;
	tstring path;
	tstring arguments;
	tstring workingDirectory;
	tstring iconPath;
	int iconIndex;
	tstring description;
	bool separator;
};

struct JumpListCategory {
	JumpListCategory() { }
	JumpListCategory(const tstring& name_) : name(name_) { }

	tstring name;
	std::vector<JumpListLink> links;
};

struct JumpList {
	JumpList() :
		showFrequent(false),
		showRecent(false)
	{
	}

	tstring appId;
	bool showFrequent;
	bool showRecent;
	std::vector<JumpListCategory> categories;
	std::vector<JumpListLink> userTasks;
};

/** provides widgets with the ability to play with the taskbar associated with a main window. */
class Taskbar {
public:
	static HRESULT setCurrentAppId(const tstring& appId);
	static HRESULT setWindowAppId(HWND window, const tstring& appId);
	static HRESULT setWindowAppId(WindowPtr window, const tstring& appId);
	static HRESULT commitJumpList(const JumpList& jumpList, UINT* minSlots = nullptr);
	static HRESULT deleteJumpList(const tstring& appId = tstring());

	void initTaskbar(FramePtr window_);

	void setOverlayIcon(CompositePtr tab, const IconPtr& icon, const tstring& description);
	void setProgressState(TBPFLAG state);
	void setProgressValue(ULONGLONG completed, ULONGLONG total);
	void addThumbnailToolbarButtons(const std::vector<THUMBBUTTON>& buttons);
	void updateThumbnailToolbarButtons(const std::vector<THUMBBUTTON>& buttons);
	void setThumbnailTooltip(const tstring& tooltip);
	void setThumbnailClip(const Rectangle& clip);
	void clearThumbnailClip();
	void addThumbnailToolbarButtons(CompositePtr tab, const std::vector<THUMBBUTTON>& buttons);
	void updateThumbnailToolbarButtons(CompositePtr tab, const std::vector<THUMBBUTTON>& buttons);
	void setThumbnailTooltip(CompositePtr tab, const tstring& tooltip);
	void setThumbnailClip(CompositePtr tab, const Rectangle& clip);
	void clearThumbnailClip(CompositePtr tab);
	void setTabProperties(CompositePtr tab, STPFLAG properties);

protected:
	Taskbar();
	virtual ~Taskbar();

	void addToTaskbar(CompositePtr tab);
	void removeFromTaskbar(CompositePtr tab);
	void moveOnTaskbar(CompositePtr tab, CompositePtr rightNeighbor = 0);
	void setActiveOnTaskbar(CompositePtr tab);
	void setTaskbarIcon(CompositePtr tab, const IconPtr& icon);

	ITaskbarList3* taskbar;
	ITaskbarList4* taskbar4;

private:
	BitmapPtr getBitmap(CompositePtr tab, LPARAM thumbnailSize);
	HWND getTaskbarWindow(CompositePtr tab) const;

	/// function called when the user activates a tab using the taskbar.
	virtual void setActive(CompositePtr) = 0;

	FramePtr window;
	std::unordered_map<CompositePtr, FramePtr> tabs;
};

}

#endif
