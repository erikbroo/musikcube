//////////////////////////////////////////////////////////////////////////////
//
// License Agreement:
//
// The following are Copyright  2007, musikCube team
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

#include "pch.hpp"
#include <cube/TransportController.hpp>

#include <cube/TransportView.hpp>
#include <core/PlaybackQueue.h>
#include <win32cpp/ApplicationThread.hpp>
#include <win32cpp/Trackbar.hpp>
#include <win32cpp/Button.hpp>
#include <win32cpp/Label.hpp>
#include <boost/format.hpp>

//////////////////////////////////////////////////////////////////////////////

using namespace musik::cube;

//////////////////////////////////////////////////////////////////////////////

/*ctor*/    TransportController::TransportController(TransportView& transportView)
: transportView(transportView)
, playbackSliderTimer(100)
, playbackSliderMouseDown(false)
, paused(false)
, playing(false)
{
    this->transportView.Created.connect(
        this, &TransportController::OnViewCreated);
    
    this->transportView.Resized.connect(
        this, &TransportController::OnViewResized);
}

void        TransportController::OnViewCreated(Window* window)
{
    this->transportView.playButton->Pressed.connect(
        this, &TransportController::OnPlayPressed);

    this->transportView.stopButton->Pressed.connect(
        this, &TransportController::OnStopPressed);

    this->transportView.nextButton->Pressed.connect(
        this, &TransportController::OnNextPressed);

    this->transportView.prevButton->Pressed.connect(
        this, &TransportController::OnPreviousPressed);

    this->transportView.volumeSlider->Repositioned.connect(
        this, &TransportController::OnVolumeSliderChange);

    this->transportView.volumeSlider->SetPosition(
        (short)(musik::core::PlaybackQueue::Instance().Transport().Volume()*(double)100));

    musik::core::PlaybackQueue::Instance().CurrentTrackChanged.connect(this,&TransportController::OnTrackChange);

    this->transportView.playbackSlider->Repositioned.connect(
        this, &TransportController::OnPlaybackSliderChange);

    this->transportView.playbackSlider->MouseButtonDown.connect(
        this, &TransportController::OnPlaybackSliderMouseDown);

    this->transportView.playbackSlider->MouseButtonUp.connect(
        this, &TransportController::OnPlaybackSliderMouseUp);

    musik::core::PlaybackQueue::Instance().Transport().PlaybackStarted.connect(this, &TransportController::OnPlaybackStarted);
    musik::core::PlaybackQueue::Instance().Transport().PlaybackEnded.connect(this, &TransportController::OnPlaybackStopped);

    musik::core::PlaybackQueue::Instance().Transport().PlaybackPause.connect(this, &TransportController::OnPlaybackPaused);
    musik::core::PlaybackQueue::Instance().Transport().PlaybackResume.connect(this, &TransportController::OnPlaybackResumed);

    this->playbackSliderTimer.ConnectToWindow(&this->transportView);

    this->playbackSliderTimer.OnTimeout.connect(this, &TransportController::OnPlaybackSliderTimerTimedOut);
    this->playbackSliderTimer.Start();

    // In case playback has already started
    if(musik::core::PlaybackQueue::Instance().CurrentTrack()){
        this->OnPlaybackStarted();
    }
}

void        TransportController::OnViewResized(Window* window, Size size)
{
}

void        TransportController::OnPlayPressed(Button* button)
{
    if (!this->playing)     musik::core::PlaybackQueue::Instance().Play();
    else if (this->paused)  musik::core::PlaybackQueue::Instance().Resume();
    else                    musik::core::PlaybackQueue::Instance().Pause();
}

void        TransportController::OnStopPressed(Button* button)
{
    musik::core::PlaybackQueue::Instance().Stop();
    this->transportView.playbackSlider->SetPosition(0);
}

void        TransportController::OnNextPressed(Button* button)
{
    musik::core::PlaybackQueue::Instance().Next();
}

void        TransportController::OnPreviousPressed(Button* button)
{
    musik::core::PlaybackQueue::Instance().Previous();
}

void        TransportController::OnVolumeSliderChange(Trackbar* trackbar)
{
    musik::core::PlaybackQueue::Instance().Transport().SetVolume( ((double)transportView.volumeSlider->Position())/100.0 );
}

void TransportController::OnTrackChange(musik::core::TrackPtr track){
    if(!win32cpp::ApplicationThread::InMainThread()){
        win32cpp::ApplicationThread::Call1(this,&TransportController::OnTrackChange,track);
        return;
    }

    if(track == musik::core::PlaybackQueue::Instance().CurrentTrack()){
        win32cpp::uistring title(_T("-"));
        win32cpp::uistring artist(_T("-"));

        if(track){

            if(track->GetValue("title"))
                title.assign( track->GetValue("title") );

            if(track->GetValue("visual_artist"))
                artist.assign( track->GetValue("visual_artist") );

            this->transportView.timeDurationLabel->SetCaption(this->FormatTime(this->CurrentTrackLength()));
        }else{
            this->transportView.timeElapsedLabel->SetCaption(_T("0:00"));
            this->transportView.timeDurationLabel->SetCaption(_T("0:00"));
        }

        this->transportView.titleLabel->SetCaption(title);
        this->transportView.artistLabel->SetCaption(artist);
    }
}

void TransportController::OnPlaybackSliderChange(Trackbar *trackBar)
{
    if(this->playbackSliderMouseDown){
        // Get the length from the track
        double trackLength  = this->CurrentTrackLength();

        if (trackLength>0){
            double relativePosition = (double)trackBar->Position() / (double)trackBar->Range();
            double newPosition      = trackLength * relativePosition;
            musik::core::PlaybackQueue::Instance().Transport().SetPosition(newPosition);
        }
    }
}

void TransportController::OnPlaybackSliderTimerTimedOut()
{
    if(!musik::core::PlaybackQueue::Instance().CurrentTrack()){
        return;
    }

    // Get the length from the track
    double trackLength  = this->CurrentTrackLength();

    // Move the slider
    double position         = musik::core::PlaybackQueue::Instance().Transport().Position();
    if (!this->playbackSliderMouseDown){
        if(trackLength>0){
            this->transportView.playbackSlider->SetPosition( (short)((double)this->transportView.playbackSlider->Range()*(position/trackLength)) );
        }
        this->transportView.timeElapsedLabel->SetCaption(this->FormatTime(position));
        return;
    }

    if (!this->playbackSliderMouseDown){
        this->transportView.playbackSlider->SetPosition(0);
    }else{
        // Lets show what position to jump to
//        double newPosition   = trackLength*(double)this->transportView.playbackSlider->Position()/(double)this->transportView.playbackSlider->Range();
//        this->transportView.timeElapsedLabel->SetCaption(this->FormatTime(newPosition));
    }

}

double  TransportController::CurrentTrackLength(){
    musik::core::TrackPtr track = musik::core::PlaybackQueue::Instance().CurrentTrack();
    if(track){
        const utfchar *totalPositionString = track->GetValue("duration");
        if(totalPositionString){
            try{
                double totalPosition    = boost::lexical_cast<double>(totalPositionString);
                return totalPosition;
            }catch(...){
            }
        }
    }
    return 0;
}

void TransportController::OnPlaybackSliderMouseDown(Window* windows, MouseEventFlags flags, Point point)
{
    this->playbackSliderMouseDown = true;
}

void TransportController::OnPlaybackSliderMouseUp(Window* windows, MouseEventFlags flags, Point point)
{
    this->playbackSliderMouseDown = false;
}

void TransportController::OnPlaybackStarted()
{
    if(!win32cpp::ApplicationThread::InMainThread())
    {
        win32cpp::ApplicationThread::Call0(this, &TransportController::OnPlaybackStarted);
        return;
    }

    musik::core::TrackPtr currentTrack  = musik::core::PlaybackQueue::Instance().CurrentTrack();

    this->playing = true;
    this->transportView.playButton->SetCaption(_T("Pause"));

    this->transportView.timeDurationLabel->SetCaption(this->FormatTime(this->CurrentTrackLength()));
    
    this->transportView.playbackSlider->SetPosition(0);
    
    this->playbackSliderTimer.Start();

    this->displayedTrack = currentTrack;
}

void TransportController::OnPlaybackStopped()
{
    if(!win32cpp::ApplicationThread::InMainThread())
    {
        win32cpp::ApplicationThread::Call0(this, &TransportController::OnPlaybackStopped);
        return;
    }

    this->displayedTrack.reset();

    this->playing = false;
    this->paused = false;

    this->transportView.playButton->SetCaption(_T("Play"));
    this->transportView.playbackSlider->SetPosition(0);

    this->OnTrackChange(musik::core::TrackPtr());
    this->transportView.timeElapsedLabel->SetCaption(_T("0:00"));

}

void TransportController::OnPlaybackPaused()
{
    if(!win32cpp::ApplicationThread::InMainThread())
    {
        win32cpp::ApplicationThread::Call0(this, &TransportController::OnPlaybackPaused);
        return;
    }

    this->paused = true;
    this->transportView.playButton->SetCaption(_(_T("Resume")));
}

void TransportController::OnPlaybackResumed()
{
    if(!win32cpp::ApplicationThread::InMainThread())
    {
        win32cpp::ApplicationThread::Call0(this, &TransportController::OnPlaybackResumed);
        return;
    }

    this->paused = false;
    this->transportView.playButton->SetCaption(_(_T("Pause")));
}

win32cpp::uistring  TransportController::FormatTime(double seconds)
{
    if(seconds>=0){
        unsigned long second    = (unsigned long)seconds % 60;
        unsigned long minutes   = (unsigned long)seconds / 60;
        
        boost::basic_format<uichar> format(_T("%1%:%2$02d"));
        format % minutes;
        format % second;
        
        return format.str();
    }
    return _(_T("Unknown"));
}
/*
win32cpp::uistring TransportController::FormatTime(const utfchar *seconds){
    if(seconds){
        try{
            double sec  = boost::lexical_cast<double>(utfstring(seconds));
            return this->FormatTime(sec);
        }catch(...){
        }
    }
    return UTF("0:00");
}*/
