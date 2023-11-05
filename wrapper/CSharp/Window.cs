/*
 * Window.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

namespace LLGL
{
    public class Window : Surface
    {
        private NativeLLGL.Window native;

        internal NativeLLGL.Window Native
        {
            get
            {
                return native;
            }
            private set
            {
                native = value;
                unsafe
                {
                    NativeLLGL.SetWindowUserData(native, (void*)(IntPtr)Id);
                }
            }
        }

        private int Id { get; set; }

        private static List<Window> windowList = new List<Window>();

        private static Window FindById(int id) => windowList.Find(window => window.Id == id);

        private static Window FindByNative(NativeLLGL.Window nativeWindow)
        {
            unsafe
            {
                return FindById((int)(IntPtr)NativeLLGL.GetWindowUserData(nativeWindow));
            }
        }

        internal Window(NativeLLGL.Window native) : base(native.AsSurface())
        {
            windowList.Add(this);
            Native = native;
        }

        ~Window()
        {
            windowList.Remove(this);
        }

        public static Window Create(WindowDescriptor windowDesc)
        {
            var nativeDesc = windowDesc.Native;
            return new Window(NativeLLGL.CreateWindow(ref nativeDesc));
        }

        public class EventListener
        {
            private NativeLLGL.OnWindowQuitDelegate OnQuitDelegate { get; set; }

            private NativeLLGL.OnWindowKeyDownDelegate OnKeyDownDelegate { get; set; }
            private NativeLLGL.OnWindowKeyUpDelegate OnKeyUpDelegate { get; set; }
            private NativeLLGL.OnWindowDoubleClickDelegate OnDoubleClickDelegate { get; set; }
            private NativeLLGL.OnWindowCharDelegate OnCharDelegate { get; set; }
            private NativeLLGL.OnWindowWheelMotionDelegate OnWheelMotionDelegate { get; set; }
            private NativeLLGL.OnWindowLocalMotionDelegate OnLocalMotionDelegate { get; set; }
            private NativeLLGL.OnWindowGlobalMotionDelegate OnGlobalMotionDelegate { get; set; }
            private NativeLLGL.OnWindowResizeDelegate OnResizeDelegate { get; set; }
            private NativeLLGL.OnWindowUpdateDelegate OnUpdateDelegate { get; set; }
            private NativeLLGL.OnWindowGetFocusDelegate OnGetFocusDelegate { get; set; }
            private NativeLLGL.OnWindowLostFocusDelegate OnLostFocusDelegate { get; set; }

            internal NativeLLGL.WindowEventListener Native { get; private set; }

            private void CreateDelegates()
            {
                unsafe
                {
                    OnQuitDelegate = (NativeLLGL.OnWindowQuitDelegate)(
                        (NativeLLGL.Window sender, bool* veto) =>
                            this.OnQuit(Window.FindByNative(sender), ref *veto));

                    OnKeyDownDelegate = (NativeLLGL.OnWindowKeyDownDelegate)(
                        (NativeLLGL.Window sender, Key keyCode) =>
                            this.OnKeyDown(Window.FindByNative(sender), keyCode));

                    OnKeyUpDelegate = (NativeLLGL.OnWindowKeyUpDelegate)(
                        (NativeLLGL.Window sender, Key keyCode) =>
                            this.OnKeyUp(Window.FindByNative(sender), keyCode));

                    OnDoubleClickDelegate = (NativeLLGL.OnWindowDoubleClickDelegate)(
                        (NativeLLGL.Window sender, Key keyCode) =>
                            this.OnDoubleClick(Window.FindByNative(sender), keyCode));

                    OnCharDelegate = (NativeLLGL.OnWindowCharDelegate)(
                        (NativeLLGL.Window sender, char chr) =>
                            this.OnChar(Window.FindByNative(sender), chr));

                    OnWheelMotionDelegate = (NativeLLGL.OnWindowWheelMotionDelegate)(
                        (NativeLLGL.Window sender, int motion) =>
                            this.OnWheelMotion(Window.FindByNative(sender), motion));

                    OnLocalMotionDelegate = (NativeLLGL.OnWindowLocalMotionDelegate)(
                        (NativeLLGL.Window sender, ref Offset2D position) =>
                            this.OnLocalMotion(Window.FindByNative(sender), position));

                    OnGlobalMotionDelegate = (NativeLLGL.OnWindowGlobalMotionDelegate)(
                        (NativeLLGL.Window sender, ref Offset2D motion) =>
                            this.OnGlobalMotion(Window.FindByNative(sender), motion));

                    OnResizeDelegate = (NativeLLGL.OnWindowResizeDelegate)(
                        (NativeLLGL.Window sender, ref Extent2D clientAreaSize) =>
                            this.OnResize(Window.FindByNative(sender), clientAreaSize));

                    OnUpdateDelegate = (NativeLLGL.OnWindowUpdateDelegate)(
                        (NativeLLGL.Window sender) =>
                            this.OnUpdate(Window.FindByNative(sender)));

                    OnGetFocusDelegate = (NativeLLGL.OnWindowGetFocusDelegate)(
                        (NativeLLGL.Window sender) =>
                            this.OnGetFocus(Window.FindByNative(sender)));

                    OnLostFocusDelegate = (NativeLLGL.OnWindowLostFocusDelegate)(
                        (NativeLLGL.Window sender) =>
                            this.OnLostFocus(Window.FindByNative(sender)));
                }
            }

            private void ForwardDelegatePointers()
            {
                var nativeFuncTable = new NativeLLGL.WindowEventListener();

                nativeFuncTable.onQuit = Marshal.GetFunctionPointerForDelegate(OnQuitDelegate);
                nativeFuncTable.onKeyDown = Marshal.GetFunctionPointerForDelegate(OnKeyDownDelegate);
                nativeFuncTable.onKeyUp = Marshal.GetFunctionPointerForDelegate(OnKeyUpDelegate);
                nativeFuncTable.onDoubleClick = Marshal.GetFunctionPointerForDelegate(OnDoubleClickDelegate);
                nativeFuncTable.onChar = Marshal.GetFunctionPointerForDelegate(OnCharDelegate);
                nativeFuncTable.onWheelMotion = Marshal.GetFunctionPointerForDelegate(OnWheelMotionDelegate);
                nativeFuncTable.onLocalMotion = Marshal.GetFunctionPointerForDelegate(OnLocalMotionDelegate);
                nativeFuncTable.onGlobalMotion = Marshal.GetFunctionPointerForDelegate(OnGlobalMotionDelegate);
                nativeFuncTable.onResize = Marshal.GetFunctionPointerForDelegate(OnResizeDelegate);
                nativeFuncTable.onUpdate = Marshal.GetFunctionPointerForDelegate(OnUpdateDelegate);
                nativeFuncTable.onGetFocus = Marshal.GetFunctionPointerForDelegate(OnGetFocusDelegate);
                nativeFuncTable.onLostFocus = Marshal.GetFunctionPointerForDelegate(OnLocalMotionDelegate);

                Native = nativeFuncTable;
            }

            public EventListener()
            {
                CreateDelegates();
                ForwardDelegatePointers();
            }

            public virtual void OnQuit(Window sender, ref bool veto) { /*dummy*/ }
            public virtual void OnKeyDown(Window sender, Key keyCode) { /*dummy*/ }
            public virtual void OnKeyUp(Window sender, Key keyCode) { /*dummy*/ }
            public virtual void OnDoubleClick(Window sender, Key keyCode) { /*dummy*/ }
            public virtual void OnChar(Window sender, char chr) { /*dummy*/ }
            public virtual void OnWheelMotion(Window sender, int motion) { /*dummy*/ }
            public virtual void OnLocalMotion(Window sender, Offset2D position) { /*dummy*/ }
            public virtual void OnGlobalMotion(Window sender, Offset2D motion) { /*dummy*/ }
            public virtual void OnResize(Window sender, Extent2D clientAreaSize) { /*dummy*/ }
            public virtual void OnUpdate(Window sender) { /*dummy*/ }
            public virtual void OnGetFocus(Window sender) { /*dummy*/ }
            public virtual void OnLostFocus(Window sender) { /*dummy*/ }
        }

        public Offset2D Position
        {
            get
            {
                var position = new Offset2D();
                NativeLLGL.GetWindowPosition(Native, ref position);
                return position;
            }
            set
            {
                var position = value;
                NativeLLGL.SetWindowPosition(Native, ref position);
            }
        }

        public Extent2D Size
        {
            get
            {
                var size = new Extent2D();
                NativeLLGL.GetWindowSize(Native, ref size, useClientArea: false);
                return size;
            }
            set
            {
                var size = value;
                NativeLLGL.SetWindowSize(Native, ref size, useClientArea: false);
            }
        }

        public Extent2D ClientAreaSize
        {
            get
            {
                var size = new Extent2D();
                NativeLLGL.GetWindowSize(Native, ref size, useClientArea: true);
                return size;
            }
            set
            {
                var size = value;
                NativeLLGL.SetWindowSize(Native, ref size, useClientArea: true);
            }
        }

        public string Title
        {
            get
            {
                unsafe
                {
                    IntPtr nullTerminatedNameLength = NativeLLGL.GetWindowTitle(Native, (IntPtr)0, null);
                    if (nullTerminatedNameLength.ToInt64() > 0)
                    {
                        int nameLength = (int)nullTerminatedNameLength - 1;
                        var nativeWindowTitle = new char[nameLength];
                        fixed (char* nativeWindowTitlePtr = nativeWindowTitle)
                        {
                            NativeLLGL.GetWindowTitle(Native, (IntPtr)nameLength, nativeWindowTitlePtr);
                        }
                        return new string(nativeWindowTitle);
                    }
                    return "";
                }
            }
            set
            {
                NativeLLGL.SetWindowTitle(Native, value);
            }
        }

        /*public WindowDescriptor Desc
        {
            get
            {
            }
            set
            {
            }
        }*/

        private class EventListenerHandle
        {
            public EventListener EventListener { get; set; }
            public int Id { get; set; }
        }
        private List<EventListenerHandle> eventListenerHandles = new List<EventListenerHandle>();

        public void AddEventListener(EventListener eventListener)
        {
            if (eventListener != null)
            {
                var handle = new EventListenerHandle();
                handle.EventListener = eventListener;
                var nativeEventListener = eventListener.Native;
                handle.Id = NativeLLGL.AddWindowEventListener(Native, ref nativeEventListener);
                eventListenerHandles.Add(handle);
            }
        }
        public void RemoveEventListener(EventListener eventListener)
        {
            if (eventListener != null)
            {
                var handle = eventListenerHandles.Find(i => i.EventListener == eventListener);
                if (handle != null)
                {
                    NativeLLGL.RemoveWindowEventListener(Native, handle.Id);
                }
            }
        }

        public bool HasQuit
        {
            get
            {
                return NativeLLGL.HasWindowQuit(Native);
            }
        }

        public bool IsShown
        {
            get
            {
                return NativeLLGL.IsWindowShown(Native);
            }
        }

        public void Show(bool show = true)
        {
            NativeLLGL.ShowWindow(Native, show);
        }

        public void PostQuit()
        {
            NativeLLGL.PostWindowQuit(Native);
        }

        public void PostKeyDown(Key keyCode)
        {
            NativeLLGL.PostWindowKeyDown(Native, keyCode);
        }

        public void PostKeyUp(Key keyCode)
        {
            NativeLLGL.PostWindowKeyUp(Native, keyCode);
        }

        public void PostDoubleClick(Key keyCode)
        {
            NativeLLGL.PostWindowDoubleClick(Native, keyCode);
        }

        public void PostChar(char chr)
        {
            NativeLLGL.PostWindowChar(Native, chr);
        }

        public void PostWheelMotion(int motion)
        {
            NativeLLGL.PostWindowWheelMotion(Native, motion);
        }

        public void PostLocalMotion(Offset2D position)
        {
            NativeLLGL.PostWindowLocalMotion(Native, ref position);
        }

        public void PostGlobalMotion(Offset2D motion)
        {
            NativeLLGL.PostWindowGlobalMotion(Native, ref motion);
        }

        public void PostResize(Extent2D clientAreaSize)
        {
            NativeLLGL.PostWindowResize(Native, ref clientAreaSize);
        }

        public void PostUpdate()
        {
            NativeLLGL.PostWindowUpdate(Native);
        }

        public void PostGetFocus()
        {
            NativeLLGL.PostWindowGetFocus(Native);
        }

        public void PostLostFocus()
        {
            NativeLLGL.PostWindowLostFocus(Native);
        }

    }
}




// ================================================================================
