//
// LibSourcey
// Copyright (C) 2005, Sourcey <http://sourcey.com>
//
// LibSourcey is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// LibSourcey is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Implemented from libjingle r116 Feb 16, 2012


#ifndef SCY_MEDIA_DeviceManager_WIN32_H
#define SCY_MEDIA_DeviceManager_WIN32_H

//#ifndef __WINDOWS_DS__
//#define __WINDOWS_DS__
//#endif

#include "scy/base.h"
#include "scy/media/devicemanager.h"

#include <string>
#include <vector>

#ifdef HAVE_RTAUDIO


namespace scy {
namespace av {


//
// Win32 Device Manager
//


class Win32DeviceManager: public DeviceManager
{
public:
    Win32DeviceManager();
    virtual ~Win32DeviceManager();

    virtual bool initialize();
    virtual void uninitialize();

    virtual bool getVideoCaptureDevices(std::vector<Device>& devs);
    virtual bool getDefaultVideoCaptureDevice(Device& device);

private:

    bool _needCoUninitialize;
    static Mutex _mutex;
};


//
// Win32 Window
//


class Win32Window
{
public:
    Win32Window();
    virtual ~Win32Window();

    bool Create(HWND parent, const wchar_t* title, DWORD style, DWORD exstyle,
        int x, int y, int cx, int cy);
    void Destroy();

    HWND handle() const { return wnd_; }

    static void Shutdown();
        // Call this when your DLL unloads.

protected:
    virtual bool OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result);

    virtual bool OnClose() { return true; }
    virtual void OnNcDestroy() { }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND wnd_;
    static HINSTANCE instance_;
    static ATOM window_class_;
};


//
// Win32 Device Watcher
//


class Win32DeviceWatcher: public DeviceWatcher, public Win32Window
    /// Plugs into windows event notifications to detect the adding
    /// and removing of attached audio and video devices.
    ///
    /// Note: The application must be compiled with SUBSYSTEM:WINDOWS.
    /// Console applications receive no device notification.
{
public:
    explicit Win32DeviceWatcher(Win32DeviceManager* dm);
    virtual ~Win32DeviceWatcher();
    virtual bool start();
    virtual void stop();

private:
    HDEVNOTIFY Register(REFGUID guid);
    void Unregister(HDEVNOTIFY notify);
    virtual bool OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT& result);

    Win32DeviceManager* manager_;
    HDEVNOTIFY audio_notify_;
    HDEVNOTIFY video_notify_;
};


} } // namespace scy::av


#endif
#endif  // SCY_MEDIA_DeviceManager_WIN32_H


/*
* libjingle
* Copyright 2004 Google Inc.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*  3. The name of the author may not be used to endorse or promote products
*     derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
