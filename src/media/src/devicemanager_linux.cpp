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


#include "scy/media/devicemanager_linux.h"
#include "scy/filesystem.h"

#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
#include <libudev.h>
#include "talk/base/linux.h"
#include "talk/base/logging.h"
#include "talk/base/fileutils.h"
#include "talk/base/pathutils.h"
#include "talk/base/physicalsocketserver.h"
#include "talk/base/stream.h"
#include "talk/base/stringutils.h"
#include "talk/base/thread.h"
#include "talk/session/phone/libudevsymboltable.h"
#include "talk/session/phone/mediacommon.h"
#include "talk/session/phone/v4llookup.h"
#include "talk/sound/platformsoundsystem.h"
#include "talk/sound/platformsoundsystemfactory.h"
#include "talk/sound/sounddevicelocator.h"
#include "talk/sound/soundsysteminterface.h"
*/
using std::endl;


namespace scy {
namespace av {


IDeviceManager* DeviceManagerFactory::create() {
    return new LinuxDeviceManager();
}


/*
class LinuxDeviceWatcher
    : public DeviceWatcher,
    private talk_base::Dispatcher {
public:
    explicit LinuxDeviceWatcher(IDeviceManager* dm);
    virtual ~LinuxDeviceWatcher();
    virtual bool start();
    virtual void stop();

private:
    virtual uint32 GetRequestedEvents();
    virtual void OnPreEvent(uint32 ff);
    virtual void OnEvent(uint32 ff, int err);
    virtual int GetDescriptor();
    virtual bool IsDescriptorClosed();

    IDeviceManager* manager_;
    LibUDevSymbolTable libudev_;
    struct udev* udev_;
    struct udev_monitor* udev_monitor_;
    bool registered_;
};

#define LATE(sym) LATESYM_GET(LibUDevSymbolTable, &libudev_, sym)
*/


static const char* const kFilteredAudioDevicesName[] = {
#if defined(CHROMEOS)
    "surround40:",
    "surround41:",
    "surround50:",
    "surround51:",
    "surround71:",
    "iec958:",      // S/PDIF
#endif
    NULL,
};
static const char* kFilteredVideoDevicesName[] = {
    NULL,
};


LinuxDeviceManager::LinuxDeviceManager()
    //: sound_system_(new PlatformSoundSystemFactory())
{
    //setWatcher(new LinuxDeviceWatcher(this));
}


LinuxDeviceManager::~LinuxDeviceManager()
{
}


// bool LinuxDeviceManager::getAudioDevices(bool input, std::vector<Device>& devs)
// {
//     devs.clear();
//
//     NEW METHOD:
//
//     if (!sound_system_.get()) {
//         return false;
//     }
//     SoundSystemInterface::SoundDeviceLocatorList list;
//     bool success;
//     if (input) {
//         success = sound_system_->EnumerateCaptureDevices(&list);
//     } else {
//         success = sound_system_->EnumeratePlaybackDevices(&list);
//     }
//     if (!success) {
//         ErrorL << "Can't enumerate devices" << endl;
//         sound_system_.release();
//         return false;
//     }
//     // We have to start the index at 1 because GIPS VoiceEngine puts the default
//     // device at index 0, but Enumerate(Capture|Playback)Devices does not include
//     // a locator for the default device.
//     int index = 1;
//     for (SoundSystemInterface::SoundDeviceLocatorList::iterator i = list.begin();
//         i != list.end();
//         ++i, ++index) {
//             devs.push_back(Device((*i)->name(), index));
//     }
//     SoundSystemInterface::ClearSoundDeviceLocatorList(&list);
//     sound_system_.release();
//
//  OLD METHOD:
//
//     int card = -1, dev = -1;
//     snd_ctl_t *handle = NULL;
//     snd_pcm_info_t *pcminfo = NULL;
//
//     snd_pcm_info_malloc(&pcminfo);
//
//     while (true) {
//         if (snd_card_next(&card) != 0 || card < 0)
//             break;
//
//         char *card_name;
//         if (snd_card_get_name(card, &card_name) != 0)
//             continue;
//
//         char card_string[7];
//         snprintf(card_string, sizeof(card_string), "hw:%d", card);
//         if (snd_ctl_open(&handle, card_string, 0) != 0)
//             continue;
//
//         while (true) {
//             if (snd_ctl_pcm_next_device(handle, &dev) < 0 || dev < 0)
//                 break;
//             snd_pcm_info_set_device(pcminfo, dev);
//             snd_pcm_info_set_subdevice(pcminfo, 0);
//             snd_pcm_info_set_stream(pcminfo, input ? SND_PCM_STREAM_CAPTURE :
//                 SND_PCM_STREAM_PLAYBACK);
//             if (snd_ctl_pcm_info(handle, pcminfo) != 0)
//                 continue;
//
//             char name[128];
//             sprintfn(name, sizeof(name), "%s (%s)", card_name,
//                 snd_pcm_info_get_name(pcminfo));
//             // TODO(tschmelcher): We might want to identify devices with something
//             // more specific than just their card number (e.g., the PCM names that
//             // aplay -L prints out).
//             devs.push_back(Device(input ? "audioin" : "audioout", name, card));
//
//             Log("debug") << "Found device: id = " << card << ", name = " << name << endl;
//         }
//         snd_ctl_close(handle);
//     }
//     snd_pcm_info_free(pcminfo);
//
//     return filterDevices(devs, kFilteredAudioDevicesName);
// }


static const std::string kVideoMetaPathK2_4("/proc/video/dev/");
static const std::string kVideoMetaPathK2_6("/sys/class/video4linux/");


enum MetaType { M2_4, M2_6, NONE };


bool IsV4L2Device(const std::string& device_path) {
  // check device major/minor numbers are in the range for video devices.
  struct stat s;
  if (lstat(device_path.c_str(), &s) != 0 || !S_ISCHR(s.st_mode)) return false;
  int video_fd = -1;
  bool is_v4l2 = false;
  // check major/minur device numbers are in range for video device
  if (major(s.st_rdev) == 81) {
    dev_t num = minor(s.st_rdev);
    if (num <= 63 && num >= 0) {
      video_fd = ::open(device_path.c_str(), O_RDONLY | O_NONBLOCK);
      if ((video_fd >= 0) || (errno == EBUSY)) {
        ::v4l2_capability video_caps;
        memset(&video_caps, 0, sizeof(video_caps));
        if ((errno == EBUSY) ||
            (::ioctl(video_fd, VIDIOC_QUERYCAP, &video_caps) >= 0 &&
            (video_caps.capabilities & V4L2_CAP_VIDEO_CAPTURE))) {
          InfoL << "Found V4L2 capture device " << device_path;
          is_v4l2 = true;
        } else {
          ErrorL << "VIDIOC_QUERYCAP failed for " << device_path;
        }
      } else {
        ErrorL << "Failed to open " << device_path;
      }
    }
  }
  if (video_fd >= 0)
    ::close(video_fd);
  return is_v4l2;
}

static std::string Trim(const std::string& s, const std::string& drop = " \t") {
    std::string::size_type first = s.find_first_not_of(drop);
    std::string::size_type last  = s.find_last_not_of(drop);

    if (first == std::string::npos || last == std::string::npos)
        return std::string("");

    return s.substr(first, last - first + 1);
}

static void ScanDeviceDirectory(const std::string& devdir,
    std::vector<Device>& devices) {

    std::vector<std::string> nodes;
    fs::readdir(devdir, nodes);
    for (unsigned i = 0; i < nodes.size(); i++) {
        std::string filename = nodes[0];
        std::string deviceName = devdir + filename;
        //if (!directoryIterator->IsDots()) {
            if (filename.find("video") == 0 &&
                IsV4L2Device(deviceName)) {
                    devices.push_back(Device("video", i, deviceName));
            }
        //}
    }

        // talk_base::scoped_ptr<talk_base::DirectoryIterator> directoryIterator(
        //     talk_base::Filesystem::IterateDirectory());
        //
        // if (directoryIterator->Iterate(talk_base::Pathname(devdir))) {
        //     do {
        //         std::string filename = directoryIterator->Name();
        //         std::string deviceName = devdir + filename;
        //         if (!directoryIterator->IsDots()) {
        //             if (filename.find("video") == 0 &&
        //                 V4LLookup::IsV4L2Device(deviceName)) {
        //                     devices.push_back(Device(deviceName, deviceName));
        //             }
        //         }
        //     } while (directoryIterator->Next());
        // }
}

/*
static std::string getVideoDeviceNameK2_6(const std::string& device_meta_path) {
    std::string deviceName;

    talk_base::scoped_ptr<talk_base::FileStream> device_meta_stream(
        talk_base::Filesystem::OpenFile(device_meta_path, "r"));

    if (device_meta_stream.get() != NULL) {
        if (device_meta_stream->ReadLine(&deviceName) != talk_base::SR_SUCCESS) {
            ErrorL << "Failed to read V4L2 device meta " << device_meta_path << endl;
        }
        device_meta_stream->Close();
    }

    return deviceName;
}

static std::string getVideoDeviceNameK2_4(const std::string& device_meta_path) {
    talk_base::ConfigParser::MapVector all_values;

    talk_base::ConfigParser config_parser;
    talk_base::FileStream* file_stream =
        talk_base::Filesystem::OpenFile(device_meta_path, "r");

    if (file_stream == NULL) return "";

    config_parser.Attach(file_stream);
    config_parser.Parse(&all_values);

    for (talk_base::ConfigParser::MapVector::iterator i = all_values.begin();
        i != all_values.end(); ++i) {
            talk_base::ConfigParser::SimpleMap::iterator deviceName_i =
                i->find("name");

            if (deviceName_i != i->end()) {
                return deviceName_i->second;
            }
    }

    return "";
}
*/

static std::string getVideoDeviceName(MetaType meta,
    const std::string& device_file_name) {
        std::string device_meta_path;
        std::string deviceName;
        std::string meta_file_path;

        // if (meta == M2_6) {
        //     meta_file_path = kVideoMetaPathK2_6 + device_file_name + "/name";
        //
        //     InfoL << "Trying " + meta_file_path << endl;
        //     deviceName = getVideoDeviceNameK2_6(meta_file_path);
        //
        //     if (deviceName.empty()) {
        //         meta_file_path = kVideoMetaPathK2_6 + device_file_name + "/model";
        //
        //         InfoL << "Trying " << meta_file_path << endl;
        //         deviceName = getVideoDeviceNameK2_6(meta_file_path);
        //     }
        // } else {
        //     meta_file_path = kVideoMetaPathK2_4 + device_file_name;
        //     InfoL << "Trying " << meta_file_path << endl;
        //     deviceName = getVideoDeviceNameK2_4(meta_file_path);
        // }

        if (deviceName.empty()) {
            deviceName = "/dev/" + device_file_name;
            ErrorL
                << "Device name not found, defaulting to device path " << deviceName << endl;
        }

        InfoL << "Name for " << device_file_name << " is " << deviceName << endl;

        return Trim(deviceName);
}

static void ScanV4L2Devices(std::vector<Device>& devices) {
    InfoL << "Enumerating V4L2 devices" << endl;

    MetaType meta;
    std::string metadata_dir;

    // talk_base::scoped_ptr<talk_base::DirectoryIterator> directoryIterator(
    //     talk_base::Filesystem::IterateDirectory());

    // Try and guess kernel version
    // if (directoryIterator->Iterate(kVideoMetaPathK2_6)) {
    //     meta = M2_6;
    //     metadata_dir = kVideoMetaPathK2_6;
    // } else if (directoryIterator->Iterate(kVideoMetaPathK2_4)) {
    //     meta = M2_4;
    //     metadata_dir = kVideoMetaPathK2_4;
    // } else {
    //     meta = NONE;
    // }
    if (fs::exists(kVideoMetaPathK2_6)) {
        meta = M2_6;
        metadata_dir = kVideoMetaPathK2_6;
    } else if (fs::exists(kVideoMetaPathK2_4)) {
        meta = M2_4;
        metadata_dir = kVideoMetaPathK2_4;
    } else {
        meta = NONE;
    }

    if (meta != NONE) {
        InfoL << "V4L2 device metadata found at " << metadata_dir << endl;

        std::vector<std::string> nodes;
        fs::readdir(metadata_dir, nodes);
        for (unsigned i = 0; i < nodes.size(); i++) {
            std::string filename = nodes[i];

            if (filename.find("video") == 0) {
                std::string device_path = "/dev/" + filename;

                if (IsV4L2Device(device_path)) {
                    devices.push_back(
                        Device("video", i, getVideoDeviceName(meta, filename), device_path));
                }
            }
        }

        // do {
        //     std::string filename = directoryIterator->Name();
        //
        //     if (filename.find("video") == 0) {
        //         std::string device_path = "/dev/" + filename;
        //
        //         if (V4LLookup::IsV4L2Device(device_path)) {
        //             devices.push_back(
        //                 Device(getVideoDeviceName(meta, filename), device_path));
        //         }
        //     }
        // } while (directoryIterator->Next());
    } else {
        ErrorL << "Unable to detect v4l2 metadata directory" << endl;
    }

    if (devices.size() == 0) {
        InfoL << "Plan B. Scanning all video devices in /dev directory" << endl;
        ScanDeviceDirectory("/dev/", devices);
    }

    InfoL << "Total V4L2 devices found : " << devices.size() << endl;
}


bool LinuxDeviceManager::getVideoCaptureDevices(std::vector<Device>& devices)
{
    devices.clear();
    ScanV4L2Devices(devices);
    //assert(0 && "implement me");
    return filterDevices(devices, kFilteredVideoDevicesName);
}


/*
LinuxDeviceWatcher::LinuxDeviceWatcher(IDeviceManager* dm)
    : DeviceWatcher(dm),
    manager_(dm),
    udev_(NULL),
    udev_monitor_(NULL),
    registered_(false) {
}


LinuxDeviceWatcher::~LinuxDeviceWatcher() {
}


bool LinuxDeviceWatcher::start() {
    // We deliberately return true in the failure paths here because libudev is
    // not a critical component of a Linux system so it may not be present/usable,
    // and we don't want to halt LinuxDeviceManager initialization in such a case.
    if (!libudev_.Load()) {
        Log("warn") << "libudev not present/usable; LinuxDeviceWatcher disabled" << endl;
            return true;
    }
    udev_ = LATE(udev_new)();
    if (!udev_) {
        ErrorL << "udev_new()" << endl;
        return true;
    }
    // The second argument here is the event source. It can be either "kernel" or
    // "udev", but "udev" is the only correct choice. Apps listen on udev and the
    // udev daemon in turn listens on the kernel.
    udev_monitor_ = LATE(udev_monitor_new_from_netlink)(udev_, "udev");
    if (!udev_monitor_) {
        ErrorL << "udev_monitor_new_from_netlink()" << endl;
        return true;
    }
    // We only listen for changes in the video devices. Audio devices are more or
    // less unimportant because receiving device change notifications really only
    // matters for broadcasting updated send/recv capabilities based on whether
    // there is at least one device available, and almost all computers have at
    // least one audio device. Also, PulseAudio device notifications don't come
    // from the udev daemon, they come from the PulseAudio daemon, so we'd only
    // want to listen for audio device changes from udev if using ALSA. For
    // simplicity, we don't bother with any audio stuff at all.
    if (LATE(udev_monitor_filter_add_match_subsystem_devtype)(udev_monitor_,
        "video4linux",
        NULL) < 0) {
            ErrorL << "udev_monitor_filter_add_match_subsystem_devtype()" << endl;
            return true;
    }
    if (LATE(udev_monitor_enable_receiving)(udev_monitor_) < 0) {
        ErrorL << "udev_monitor_enable_receiving()" << endl;
        return true;
    }
    static_cast<talk_base::PhysicalSocketServer*>(
        talk_base::Thread::Current()->socketserver())->Add(this);
    registered_ = true;
    return true;
}


void LinuxDeviceWatcher::stop() {
    if (registered_) {
        static_cast<talk_base::PhysicalSocketServer*>(
            talk_base::Thread::Current()->socketserver())->Remove(this);
        registered_ = false;
    }
    if (udev_monitor_) {
        LATE(udev_monitor_unref)(udev_monitor_);
        udev_monitor_ = NULL;
    }
    if (udev_) {
        LATE(udev_unref)(udev_);
        udev_ = NULL;
    }
    libudev_.Unload();
}


uint32 LinuxDeviceWatcher::GetRequestedEvents() {
    return talk_base::DE_READ;
}


void LinuxDeviceWatcher::OnPreEvent(uint32 ff) {
    // Nothing to do.
}


void LinuxDeviceWatcher::OnEvent(uint32 ff, int err) {
    udev_device* device = LATE(udev_monitor_receive_device)(udev_monitor_);
    if (!device) {
        // Probably the socket connection to the udev daemon was terminated (perhaps
        // the daemon crashed or is being restarted?).
        LOG_ERR(LS_WARNING) << "udev_monitor_receive_device()";
        // Stop listening to avoid potential livelock (an fd with EOF in it is
        // always considered readable).
        static_cast<talk_base::PhysicalSocketServer*>(
            talk_base::Thread::Current()->socketserver())->Remove(this);
        registered_ = false;
        return;
    }
    // Else we read the device successfully.

    // Since we already have our own filesystem-based device enumeration code, we
    // simply re-enumerate rather than inspecting the device event.
    LATE(udev_device_unref)(device);
    manager_->SignalDevicesChange();
}


int LinuxDeviceWatcher::GetDescriptor() {
    return LATE(udev_monitor_get_fd)(udev_monitor_);
}


bool LinuxDeviceWatcher::IsDescriptorClosed() {
    // If it is closed then we will just get an error in
    // udev_monitor_receive_device and unregister, so we don't need to check for
    // it separately.
    return false;
}
*/


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
