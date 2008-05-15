//////////////////////////////////////////////////////////////////////////////
//
// License Agreement:
//
// The following are Copyright � 2007, Casey Langen
//
// Sources and Binaries of: mC2, win32cpp
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright 
//      notice, this list of conditions and the following disclaimer in the 
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the author nor the names of other contributors may 
//      be used to endorse or promote products derived from this software 
//      without specific prior written permission. 
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE. 
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////

#include <core/PlaybackQueue.h>
#include <cube/TransportView.hpp>
#include <win32cpp/Timer.hpp>

//////////////////////////////////////////////////////////////////////////////

using namespace win32cpp;

namespace musik { namespace cube {

//////////////////////////////////////////////////////////////////////////////

class TransportController : public EventHandler
{
public:     
    /*ctor*/    TransportController(TransportView& transportView);

protected:
    void        OnViewCreated(Window* window);
    void        OnViewResized(Window* window, Size size);

    void    OnPlayPressed(Button* button);
    void    OnStopPressed(Button* button);
    void    OnNextPressed(Button* button);
    void    OnPreviousPressed(Button* button);
    void    OnVolumeSliderChange(Trackbar* trackbar);
    void    OnTrackChange(musik::core::TrackPtr track);
    void    OnPlaybackSliderChange(Trackbar* trackBar);
protected:  void    OnPlaybackSliderTimerTimedOut();
protected:  void    OnPlaybackSliderMouseDown(Window* windows, MouseEventFlags flags, Point point);
protected:  void    OnPlaybackSliderMouseUp(Window* windows, MouseEventFlags flags, Point point);

    void    OnPlaybackStarted();
    void    OnPlaybackStopped();
    void    OnPlaybackPaused();
    void    OnPlaybackResumed(); 

    TransportView& transportView;

    bool    paused;
    bool    playing;

protected:  Timer   playbackSliderTimer;
protected:  bool    playbackSliderMouseDown;

protected:  win32cpp::uistring FormatTime(unsigned long ms);
};

//////////////////////////////////////////////////////////////////////////////

} }     // musik::cube
