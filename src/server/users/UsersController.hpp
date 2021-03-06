//////////////////////////////////////////////////////////////////////////////
//
// License Agreement:
//
// The following are Copyright � 2007, mC2 Team
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
// Forward declare
namespace musik { namespace server { namespace users {
    class UsersView;
    class UsersModel;
    class UsersListController;
} } }
namespace win32cpp{
    class Window;
    class Button;
}
//////////////////////////////////////////////////////////////////////////////

#include <core/Server.h>
#include <win32cpp/Types.hpp>
#include <win32cpp/WindowGeometry.hpp>
#include <boost/shared_ptr.hpp>

namespace musik { namespace server{ namespace users {

//////////////////////////////////////////////////////////////////////////////

class UsersController : public win32cpp::EventHandler
{
    public:     
        UsersController(UsersView& usersView,musik::core::Server *server);

    private:  
        void        OnViewCreated(win32cpp::Window* window);
        void        OnViewResized(win32cpp::Window* window, win32cpp::Size size);

        UsersView& usersView;

        void OnAddUser(win32cpp::Button* button);
        void OnRemoveUser(win32cpp::Button* button);

        typedef boost::shared_ptr<UsersListController> UsersListControllerRef;
        UsersListControllerRef usersListController;
    public:
        musik::core::Server *server;
};

//////////////////////////////////////////////////////////////////////////////

} } }    // musik::server
