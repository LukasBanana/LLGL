/*
 * MacroUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MACRO_UTILS_H
#define LLGL_MACRO_UTILS_H


#define LLGL_COMPARE_SEPARATE_MEMBERS_SWO(LHS, RHS) \
    if ((LHS) < (RHS)) { return -1; }               \
    if ((LHS) > (RHS)) { return +1; }

#define LLGL_COMPARE_SEPARATE_BOOL_MEMBER_SWO(LHS, RHS) \
    if (!(LHS) &&  (RHS)) { return -1; }      \
    if ( (LHS) && !(RHS)) { return +1; }

#define LLGL_COMPARE_MEMBER_SWO(MEMBER) \
    LLGL_COMPARE_SEPARATE_MEMBERS_SWO(lhs.MEMBER, rhs.MEMBER)

#define LLGL_COMPARE_BOOL_MEMBER_SWO(MEMBER) \
    LLGL_COMPARE_SEPARATE_BOOL_MEMBER_SWO(lhs.MEMBER, rhs.MEMBER)

#define LLGL_COMPARE_MEMBER_EQ(MEMBER) \
    (lhs.MEMBER == rhs.MEMBER)

#define LLGL_CASE_TO_STR(VALUE) \
    case VALUE: return #VALUE

#define LLGL_CASE_TO_STR_TYPED(TYPE, VALUE) \
    case TYPE::VALUE: return #VALUE

#define LLGL_VS_STAGE(FLAGS)        ( ((FLAGS) & StageFlags::VertexStage        ) != 0 )
#define LLGL_HS_STAGE(FLAGS)        ( ((FLAGS) & StageFlags::TessControlStage   ) != 0 )
#define LLGL_DS_STAGE(FLAGS)        ( ((FLAGS) & StageFlags::TessEvaluationStage) != 0 )
#define LLGL_GS_STAGE(FLAGS)        ( ((FLAGS) & StageFlags::GeometryStage      ) != 0 )
#define LLGL_PS_STAGE(FLAGS)        ( ((FLAGS) & StageFlags::FragmentStage      ) != 0 )
#define LLGL_CS_STAGE(FLAGS)        ( ((FLAGS) & StageFlags::ComputeStage       ) != 0 )
#define LLGL_GRAPHICS_STAGE(FLAGS)  ( ((FLAGS) & StageFlags::AllGraphicsStages  ) != 0 )

#define LLGL_VA_ARGS(...) \
    , ## __VA_ARGS__

#define LLGL_ARRAY_LENGTH(ARRAY) \
    (sizeof(ARRAY)/sizeof((ARRAY)[0]))


#endif



// ================================================================================
