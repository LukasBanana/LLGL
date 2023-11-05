/*
 * BlendDescriptor.cs
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

using System;
using System.Text;

namespace LLGL
{
    public class BlendDescriptor
    {
        public bool AlphaToCoverageEnabled { get; set; } = false;
        public bool IndependentBlendEnabled { get; set; } = false;
        public int SampleMask { get; set; } = -1;
        public LogicOp LogicOp { get; set; } = LogicOp.Disabled;
        public Color BlendFactor { get; set; } = Color.TransparentBlack;
        public bool BlendFactorDynamic { get; set; } = false;
        public BlendTargetDescriptorArray Targets { get; private set; } = new BlendTargetDescriptorArray();

        internal NativeLLGL.BlendDescriptor Native
        {
            get
            {
                var native = new NativeLLGL.BlendDescriptor();
                unsafe
                {
                    native.alphaToCoverageEnabled = AlphaToCoverageEnabled;
                    native.independentBlendEnabled = IndependentBlendEnabled;
                    native.sampleMask = (int)SampleMask;
                    native.logicOp = LogicOp;
                    native.blendFactor[0] = BlendFactor.R;
                    native.blendFactor[1] = BlendFactor.G;
                    native.blendFactor[2] = BlendFactor.B;
                    native.blendFactor[3] = BlendFactor.A;
                    native.blendFactorDynamic = BlendFactorDynamic;
                    if (IndependentBlendEnabled)
                    {
                        NativeLLGL.BlendTargetDescriptor* nativeTargets = &native.targets0;
                        for (int i = 0; i < Targets.Length && Targets[i] != null; ++i)
                        {
                            nativeTargets[i] = Targets[i].Native;
                        }
                    }
                    else if (Targets[0] != null)
                    {
                        native.targets0 = Targets[0].Native;
                    }
                }
                return native;
            }
        }
    }
}




// ================================================================================
