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


#include "scy/media/devicemanager.h"

#ifdef HAVE_RTAUDIO
#include "RtAudio.h"
#endif


using std::endl;


namespace scy {
namespace av {

static const char* kFilteredAudioDevicesName[] = {
    NULL,
};

// Initialize to empty string
const char IDeviceManager::kDefaultDeviceName[] = "";


//
// Device
//


Device::Device() :
    id(-1)
{
}


Device::Device(const std::string& type, int id, const std::string& name, const std::string& guid, bool isDefault, bool isAvailable) :
    type(type), id(id), name(name), guid(guid), isDefault(isDefault), isAvailable(isAvailable)
{
}


void Device::print(std::ostream& os)
{
    os << "Device["
        << type << ": "
        << id << ": "
        << name << ": "
        << isDefault  << ": "
        << isAvailable
        << "]";
}


//
// Device Manager
//


DeviceManager::DeviceManager() :
    _watcher(nullptr),
    _initialized(false)
{
}


DeviceManager::~DeviceManager()
{
    if (initialized())
        uninitialize();
    if (_watcher)
        delete _watcher;
}


bool DeviceManager::initialize()
{
    if (!initialized()) {
        if (watcher() && !watcher()->start())
            return false;
        setInitialized(true);
    }
    return true;
}


void DeviceManager::uninitialize()
{
    if (initialized()) {
        if (watcher())
            watcher()->stop();
        setInitialized(false);
    }
}


int DeviceManager::getCapabilities()
{
    std::vector<Device> devices;
    int caps = VIDEO_RECV;
    if (getAudioInputDevices(devices) && !devices.empty()) {
        caps |= AUDIO_SEND;
    }
    if (getAudioOutputDevices(devices) && !devices.empty()) {
        caps |= AUDIO_RECV;
    }
    if (getVideoCaptureDevices(devices) && !devices.empty()) {
        caps |= VIDEO_SEND;
    }
    return caps;
}


bool DeviceManager::getAudioInputDevices(std::vector<Device>& devices)
{
    return getAudioDevices(true, devices);
}


bool DeviceManager::getAudioOutputDevices(std::vector<Device>& devices)
{
    return getAudioDevices(false, devices);
}


bool DeviceManager::getAudioInputDevice(Device& out, const std::string& name, int id)
{
    return getAudioDevice(true, out, name, id);
}


bool DeviceManager::getAudioInputDevice(Device& out, int id)
{
    return getAudioDevice(true, out, id);
}


bool DeviceManager::getAudioOutputDevice(Device& out, const std::string& name, int id)
{
    return getAudioDevice(false, out, name, id);
}


bool DeviceManager::getAudioOutputDevice(Device& out, int id)
{
    return getAudioDevice(false, out, id);
}


bool DeviceManager::getVideoCaptureDevices(std::vector<Device>& devices)
{
    devices.clear();
#if defined(ANDROID) || defined(IOS)
    // TODO: Incomplete. Use ANDROID implementation for IOS to quiet compiler.
    // On Android, we treat the camera(s) as a single device. Even if there are
    // multiple cameras, that's abstracted away at a higher level.
    Device dev("camera", "1");    // name and ID
    devices.push_back(dev);
#else
    return false;
#endif
}


#if 0
bool DeviceManager::getAudioInputDevice(Device& out, const std::string& name, int id)
{
    // If the name is empty, return the default device.
    if (name.empty())
        return getDefaultAudioInputDevice(out);

    std::vector<Device> devices;
    getAudioInputDevices(devices);
    return matchNameAndID(devices, out, name, id);
}


bool DeviceManager::getAudioOutputDevice(Device& out, const std::string& name, int id)
{
    // If the name is empty, return the default device.
    if (name.empty())
        return getDefaultAudioOutputDevice(out);

    std::vector<Device> devices;
    return getAudioOutputDevices(devices) &&
        matchNameAndID(devices, name, id);
}
#endif


bool DeviceManager::getVideoCaptureDevice(Device& out, int id)
{
    std::vector<Device> devices;
    return getVideoCaptureDevices(devices) &&
        matchID(devices, out, id);
}


bool DeviceManager::getVideoCaptureDevice(Device& out, const std::string& name, int id)
{
    // If the name is empty, return the default device.
    if (name.empty() || name == kDefaultDeviceName) {
        return getDefaultVideoCaptureDevice(out);
    }

    std::vector<Device> devices;
    return getVideoCaptureDevices(devices) &&
        matchNameAndID(devices, out, name, id);

#if 0
    for (std::vector<Device>::const_iterator it = devices.begin(); it != devices.end(); ++it) {
        if (name == it->name) {
            InfoL << "Create VideoCapturer for " << name << endl;
            out = *it;
            return true;
        }
    }

    // If the name is a valid path to a file, then we'll create a simulated device
    // with the filename. The LmiMediaEngine will know to use a FileVideoCapturer
    // for these devices.
    if (talk_base::FileSystem::IsFile(name)) {
        InfoL << "Create FileVideoCapturer" << endl;
        *out = FileVideoCapturer::CreateFileVideoCapturerDevice(name);
        return true;
    }
#endif
}


bool DeviceManager::getAudioDevices(bool input, std::vector<Device>& devs)
{
    devs.clear();

#if defined(ANDROID)
    // Under Android, we don't access the device file directly.
    // Arbitrary use 0 for the mic and 1 for the output.
    // These ids are used in MediaEngine::SetSoundDevices(in, out);
    // The strings are for human consumption.
    if (input) {
        devs.push_back(Device("audioin", "audiorecord", 0));
    } else {
        devs.push_back(Device("audioout", "audiotrack", 1));
    }
    return true;
#elif defined(HAVE_RTAUDIO)
TraceLS(this) << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!Get audio devices: " << endl;

    // Since we are using RtAudio for audio capture it's best to
    // use RtAudio to enumerate devices to ensure indexes match.
    RtAudio audio;

    // Determine the number of devices available
    auto ndevices = audio.getDeviceCount();
    TraceLS(this) << "Get audio devices: " << ndevices << endl;

    // Scan through devices for various capabilities
    RtAudio::DeviceInfo info;
    for (unsigned i = 0; i <= ndevices; i++) {
        try {
            info = audio.getDeviceInfo(i);    // may throw RtAudioError

            TraceLS(this) << "Device:"
                << "\n\tName: " << info.name
                << "\n\tOutput Channels: " << info.outputChannels
                << "\n\tInput Channels: " << info.inputChannels
                << "\n\tDuplex Channels: " << info.duplexChannels
                << "\n\tDefault Output: " << info.isDefaultOutput
                << "\n\tDefault Input: " << info.isDefaultInput
                << "\n\tProbed: " << info.probed
                << endl;

            if (info.probed == true && (
                (input && info.inputChannels > 0) ||
                (!input && info.outputChannels > 0))) {

                TraceLS(this) << "Adding device: " << info.name << endl;
                Device dev((input ? "audioin" : "audioout"), i, info.name, "",
                    (input ? info.isDefaultInput : info.isDefaultOutput));
                devs.push_back(dev);
            }
        }
        catch (RtAudioError& e) {
            ErrorLS(this) << "Cannot probe audio device: " << e.getMessage() << endl;
        }
    }

    return filterDevices(devs, kFilteredAudioDevicesName);
#endif
}


bool DeviceManager::getDefaultVideoCaptureDevice(Device& device)
{
    bool ret = false;
    // We just return the first device.
    std::vector<Device> devices;
    ret = (getVideoCaptureDevices(devices) && !devices.empty());
    if (ret) {
        device = devices[0];
    }
    return ret;
}


bool DeviceManager::getAudioDevice(bool input, Device& out, const std::string& name, int id)
{
    TraceL << "Get audio device: " << id << ": " << name << endl;

    // If the name is empty, return the default device id.
    if (name.empty() || name == kDefaultDeviceName) {
        //out = Device(input ? "audioin" : "audioout", -1, name);
        //return true;
        //input ? getDefaultAudioInputDevice(out) : getDefaultAudioOutputDevice(out);
        return getAudioDevice(input, out, id);
    }

    std::vector<Device> devices;
    input ? getAudioInputDevices(devices) : getAudioOutputDevices(devices);
    TraceL << "Get audio devices: " << devices.size() << endl;
    return matchNameAndID(devices, out, name, id);
}


bool DeviceManager::getAudioDevice(bool input, Device& out, int id)
{
    std::vector<Device> devices;
    input ? getAudioInputDevices(devices) : getAudioOutputDevices(devices);
    TraceL << "Get audio devices: " << devices.size() << endl;
    return matchID(devices, out, id);
}


#if 0
bool DeviceManager::getDefaultAudioInputDevice(Device& device)
{
    bool ret = false;
    // We just return the first device.
    std::vector<Device> devices;
    ret = (getAudioInputDevices(devices) && !devices.empty());
    if (ret) {
        device = devices[0];
    }
    return ret;
}


bool DeviceManager::getDefaultAudioOutputDevice(Device& device)
{
    bool ret = false;
    // We just return the first device.
    std::vector<Device> devices;
    ret = (getAudioOutputDevices(devices) && !devices.empty());
    if (ret) {
        device = devices[0];
    }
    return ret;
}
#endif


bool DeviceManager::getDefaultAudioDevice(bool input, Device& device)
{
    bool ret = false;
    std::vector<Device> devices;
    ret = (getAudioDevices(input, devices) && !devices.empty());
    if (ret) {
        // Use the first device by default
        device = devices[0];

        // Loop through devices to check if any are explicitly
        // set as default
        for (size_t i = 0; i < devices.size(); ++i) {
            if (devices[i].isDefault) {
                device = devices[i];
            }
        }
    }
    return ret;
}


bool DeviceManager::getDefaultAudioInputDevice(Device& device)
{
    return getDefaultAudioDevice(true, device);
}


bool DeviceManager::getDefaultAudioOutputDevice(Device& device)
{
    return getDefaultAudioDevice(false, device);
}

// bool DeviceManager::getDefaultAudioInputDevice(Device& device)
// {
//     //device = Device("audioin", -1, "None");
//     //return true;
//     return getDefaultAudioDevice(true, device);
// }
//
//
// bool DeviceManager::getDefaultAudioOutputDevice(Device& device)
// {
//     //device = Device("audioout", -1, "None");
//     //return true;
//     return getDefaultAudioDevice(false, device);
// }


bool DeviceManager::shouldDeviceBeIgnored(const std::string& deviceName, const char* const exclusionList[])
{
    // If exclusionList is empty return directly.
    if (!exclusionList)
        return false;

    int i = 0;
    while (exclusionList[i]) {
        if (util::icompare(deviceName, exclusionList[i]) == 0) {
            TraceL << "Ignoring device " << deviceName << endl;
            return true;
        }
        ++i;
    }
    return false;
}


bool DeviceManager::filterDevices(std::vector<Device>& devices, const char* const exclusionList[])
{
    //if (!devices) {
    //    return false;
    //}

    for (auto it = devices.begin(); it != devices.end(); ) {
            if (shouldDeviceBeIgnored(it->name, exclusionList)) {
                it = devices.erase(it);
            } else {
                ++it;
            }
    }
    return true;
}


bool DeviceManager::matchID(std::vector<Device>& devices, Device& out, int id)
{
    for (unsigned i = 0; i < devices.size(); ++i) {
        if (devices[i].id == id) {
            out = devices[i];
            return true;
        }
    }

    //if (id >= 0 && !devices.empty() && devices.size() >= (id + 1)) {
    //    out = devices[id];
    //    return true;
    //}
    return false;
}


bool DeviceManager::matchNameAndID(std::vector<Device>& devices, Device& out, const std::string& name, int id)
{
    TraceL << "Match name and ID: " << name << ": " << id << endl;

    bool ret = false;
    for (int i = 0; i < static_cast<int>(devices.size()); ++i) {
        TraceL << "Match name and ID: Checking: " << devices[i].name << endl;
        if (devices[i].name == name) {
            // The first device matching the given name will be returned,
            // but we will try and match the given ID as well.
            //if (out.id == -1)
            out = devices[i];
            TraceL << "Match name and ID: Match: " << out.name << endl;

            ret = true;
            if (id == -1 || id == i) {

                TraceL << "Match name and ID: Match ID: " << out.name << endl;
                break;
            }
        }
    }

    return ret;
}


void DeviceManager::setWatcher(DeviceWatcher* watcher)
{
    if (_watcher)
        delete _watcher;
    _watcher = watcher;
    //_watcher.reset(watcher);
}


DeviceWatcher* DeviceManager::watcher()
{
    return _watcher; //_watcher.get();
}


void DeviceManager::setInitialized(bool initialized)
{
    _initialized = initialized;
}


void DeviceManager::print(std::ostream& ost)
{
    std::vector<Device> devs;
    getAudioInputDevices(devs);
    ost << "Audio input devices: " << endl;
    for (size_t i = 0; i < devs.size(); ++i)
        devs[i].print(ost);

    getAudioOutputDevices(devs);
    ost << "Audio output devices: " << endl;
    for (size_t i = 0; i < devs.size(); ++i)
        devs[i].print(ost);

    getVideoCaptureDevices(devs);
    ost << "Video capture devices: " << endl;
    for (size_t i = 0; i < devs.size(); ++i)
        devs[i].print(ost);
}


} } // namespace scy::av


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
