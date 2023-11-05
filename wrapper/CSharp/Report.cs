/*
 * RenderSystem.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;

namespace LLGL
{
    public class Report
    {
        private bool isWeakRef = false;

        internal NativeLLGL.Report Native { get; private set; }

        public Report()
        {
            Native = NativeLLGL.AllocReport();
        }

        ~Report()
        {
            if (!isWeakRef)
            {
                NativeLLGL.FreeReport(Native);
            }
        }

        internal Report(NativeLLGL.Report native)
        {
            Native = native;
            isWeakRef = true;
        }

        public bool HasErrors
        {
            get
            {
                return NativeLLGL.HasReportErrors(Native);
            }
        }

        public void Printf(string format)
        {
            var writer = new System.IO.StringWriter();
            writer.WriteLine(format);
            NativeLLGL.ResetReport(Native, ToString() + writer.ToString(), HasErrors);
        }

        public void Printf(string format, params object[] args)
        {
            var writer = new System.IO.StringWriter();
            writer.WriteLine(format, args);
            NativeLLGL.ResetReport(Native, ToString() + writer.ToString(), HasErrors);
        }

        public void Errorf(string format)
        {
            var writer = new System.IO.StringWriter();
            writer.WriteLine(format);
            NativeLLGL.ResetReport(Native, ToString() + writer.ToString(), true);
        }

        public void Errorf(string format, params object[] args)
        {
            var writer = new System.IO.StringWriter();
            writer.WriteLine(format, args);
            NativeLLGL.ResetReport(Native, ToString() + writer.ToString(), true);
        }

        public void Reset(string text, bool hasErrors)
        {
            NativeLLGL.ResetReport(Native, text, hasErrors);
        }

        public override string ToString()
        {
            return NativeLLGL.GetReportText(Native);
        }

    }
}




// ================================================================================
