; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 57
; Schema: 0
               OpCapability Shader
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
               OpExtension "SPV_GOOGLE_user_type"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %VS "main" %in_var_POSITION %in_var_TEXCOORD %in_var_COLOR %in_var_WMATRIX %in_var_ARRAYLAYER %gl_Position %out_var_WORLDPOS %out_var_TEXCOORD %out_var_COLOR
               OpSource HLSL 600
               OpName %type_Settings "type.Settings"
               OpMemberName %type_Settings 0 "vpMatrix"
               OpMemberName %type_Settings 1 "viewPos"
               OpMemberName %type_Settings 2 "fogColor"
               OpMemberName %type_Settings 3 "fogDensity"
               OpMemberName %type_Settings 4 "animVec"
               OpName %Settings "Settings"
               OpName %in_var_POSITION "in.var.POSITION"
               OpName %in_var_TEXCOORD "in.var.TEXCOORD"
               OpName %in_var_COLOR "in.var.COLOR"
               OpName %in_var_WMATRIX "in.var.WMATRIX"
               OpName %in_var_ARRAYLAYER "in.var.ARRAYLAYER"
               OpName %out_var_WORLDPOS "out.var.WORLDPOS"
               OpName %out_var_TEXCOORD "out.var.TEXCOORD"
               OpName %out_var_COLOR "out.var.COLOR"
               OpName %VS "VS"
               OpDecorateString %in_var_POSITION UserSemantic "POSITION"
               OpDecorateString %in_var_TEXCOORD UserSemantic "TEXCOORD"
               OpDecorateString %in_var_COLOR UserSemantic "COLOR"
               OpDecorateString %in_var_WMATRIX UserSemantic "WMATRIX"
               OpDecorateString %in_var_ARRAYLAYER UserSemantic "ARRAYLAYER"
               OpDecorate %gl_Position BuiltIn Position
               OpDecorateString %gl_Position UserSemantic "SV_Position"
               OpDecorateString %out_var_WORLDPOS UserSemantic "WORLDPOS"
               OpDecorateString %out_var_TEXCOORD UserSemantic "TEXCOORD"
               OpDecorateString %out_var_COLOR UserSemantic "COLOR"
               OpDecorate %in_var_POSITION Location 0
               OpDecorate %in_var_TEXCOORD Location 1
               OpDecorate %in_var_COLOR Location 2
               OpDecorate %in_var_WMATRIX Location 3
               OpDecorate %in_var_ARRAYLAYER Location 7
               OpDecorate %out_var_WORLDPOS Location 0
               OpDecorate %out_var_TEXCOORD Location 1
               OpDecorate %out_var_COLOR Location 2
               OpDecorate %Settings DescriptorSet 0
               OpDecorate %Settings Binding 2
               OpMemberDecorate %type_Settings 0 Offset 0
               OpMemberDecorate %type_Settings 0 MatrixStride 16
               OpMemberDecorate %type_Settings 0 RowMajor
               OpMemberDecorate %type_Settings 1 Offset 64
               OpMemberDecorate %type_Settings 2 Offset 80
               OpMemberDecorate %type_Settings 3 Offset 92
               OpMemberDecorate %type_Settings 4 Offset 96
               OpDecorate %type_Settings Block
               OpDecorateString %Settings UserTypeGOOGLE "cbuffer"
        %int = OpTypeInt 32 1
      %int_4 = OpConstant %int 4
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %v4float = OpTypeVector %float 4
%mat4v4float = OpTypeMatrix %v4float 4
    %v3float = OpTypeVector %float 3
    %v2float = OpTypeVector %float 2
%type_Settings = OpTypeStruct %mat4v4float %v4float %v3float %float %v2float
%_ptr_Uniform_type_Settings = OpTypePointer Uniform %type_Settings
%_ptr_Input_v3float = OpTypePointer Input %v3float
%_ptr_Input_v2float = OpTypePointer Input %v2float
%_ptr_Input_mat4v4float = OpTypePointer Input %mat4v4float
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Output_v3float = OpTypePointer Output %v3float
       %void = OpTypeVoid
         %30 = OpTypeFunction %void
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
   %Settings = OpVariable %_ptr_Uniform_type_Settings Uniform
%in_var_POSITION = OpVariable %_ptr_Input_v3float Input
%in_var_TEXCOORD = OpVariable %_ptr_Input_v2float Input
%in_var_COLOR = OpVariable %_ptr_Input_v3float Input
%in_var_WMATRIX = OpVariable %_ptr_Input_mat4v4float Input
%in_var_ARRAYLAYER = OpVariable %_ptr_Input_float Input
%gl_Position = OpVariable %_ptr_Output_v4float Output
%out_var_WORLDPOS = OpVariable %_ptr_Output_v4float Output
%out_var_TEXCOORD = OpVariable %_ptr_Output_v3float Output
%out_var_COLOR = OpVariable %_ptr_Output_v3float Output
         %VS = OpFunction %void None %30
         %33 = OpLabel
         %34 = OpLoad %v3float %in_var_POSITION
         %35 = OpLoad %v2float %in_var_TEXCOORD
         %36 = OpLoad %v3float %in_var_COLOR
         %37 = OpLoad %mat4v4float %in_var_WMATRIX
         %38 = OpLoad %float %in_var_ARRAYLAYER
         %39 = OpAccessChain %_ptr_Uniform_v2float %Settings %int_4
         %40 = OpLoad %v2float %39
         %41 = OpCompositeExtract %float %34 1
         %42 = OpVectorTimesScalar %v2float %40 %41
         %43 = OpCompositeExtract %float %34 0
         %44 = OpCompositeExtract %float %42 0
         %45 = OpFAdd %float %43 %44
         %46 = OpCompositeExtract %float %34 2
         %47 = OpCompositeExtract %float %42 1
         %48 = OpFAdd %float %46 %47
         %49 = OpCompositeConstruct %v4float %45 %41 %48 %float_1
         %50 = OpMatrixTimesVector %v4float %37 %49
         %51 = OpAccessChain %_ptr_Uniform_mat4v4float %Settings %int_0
         %52 = OpLoad %mat4v4float %51
         %53 = OpVectorTimesMatrix %v4float %50 %52
         %54 = OpCompositeExtract %float %35 0
         %55 = OpCompositeExtract %float %35 1
         %56 = OpCompositeConstruct %v3float %54 %55 %38
               OpStore %gl_Position %53
               OpStore %out_var_WORLDPOS %50
               OpStore %out_var_TEXCOORD %56
               OpStore %out_var_COLOR %36
               OpReturn
               OpFunctionEnd
