/*
 * HelperMacros.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_HELPER_MACROS_H
#define LLGL_HELPER_MACROS_H


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

#define LLGL_VS_STAGE(FLAGS) ( ((FLAGS) & StageFlags::VertexStage        ) != 0 )
#define LLGL_HS_STAGE(FLAGS) ( ((FLAGS) & StageFlags::TessControlStage   ) != 0 )
#define LLGL_DS_STAGE(FLAGS) ( ((FLAGS) & StageFlags::TessEvaluationStage) != 0 )
#define LLGL_GS_STAGE(FLAGS) ( ((FLAGS) & StageFlags::GeometryStage      ) != 0 )
#define LLGL_PS_STAGE(FLAGS) ( ((FLAGS) & StageFlags::FragmentStage      ) != 0 )
#define LLGL_CS_STAGE(FLAGS) ( ((FLAGS) & StageFlags::ComputeStage       ) != 0 )


#endif



// ================================================================================
