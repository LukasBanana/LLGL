/*
 * SpirvReflect.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SPIRV_REFLECT_H
#define LLGL_SPIRV_REFLECT_H


#include "SpirvIterator.h"
#include "SpirvModule.h"
#include <vector>
#include <map>


namespace LLGL
{


// Helper class to hold SPIR-V name decorations.
class SpirvNameDecorations
{

    public:

        SpirvNameDecorations() = default;

        inline SpirvNameDecorations(std::uint32_t idBound)
        {
            Reset(idBound);
        }

        inline void Reset(std::uint32_t idBound)
        {
            names_.clear();
            names_.resize(idBound);
        }

        inline const char* Get(spv::Id id) const
        {
            return (id < names_.size() ? names_[id] : "");
        }

        inline void Set(spv::Id id, const char* name)
        {
            if (id < names_.size())
                names_[id] = name;
        }

    public:

        inline const char* operator [] (spv::Id id) const
        {
            return Get(id);
        }

    private:

        std::vector<const char*> names_;

};

// SPIR-V shader module parser.
class SpirvReflect
{

    public:

        // Execution mode container.
        struct SpvExecutionMode
        {
            bool            earlyFragmentTest   = false;
            bool            originUpperLeft     = false;
            bool            depthGreater        = false;
            bool            depthLess           = false;
            std::uint32_t   localSizeX          = 0;
            std::uint32_t   localSizeY          = 0;
            std::uint32_t   localSizeZ          = 0;
        };

        // General purpose structure for all SPIR-V module types.
        struct SpvType
        {
            const SpvType* Deref() const;
            const SpvType* Deref(const spv::Op opcodeType) const;
            bool RefersToType(const spv::Op opcodeType) const;

            spv::Op                     opcode      = spv::OpMax;           // Opcode for this type (e.g. spv::Op::OpTypeFloat).
            spv::Id                     result      = 0;                    // Result ID of this type.
            spv::StorageClass           storage     = spv::StorageClassMax; // Storage class of this type. By default spv::StorageClass::Max.
            const char*                 name        = nullptr;              // Name of this type (only for structures).
            const SpvType*              baseType    = nullptr;              // Reference to the base type, or null if there is no base type.
            std::uint32_t               elements    = 0;                    // Number of elements for the base type, or 0 if there is no base type.
            std::uint32_t               size        = 0;                    // Size (in bytes) of this type, or 0 if this is an OpTypeVoid type.
            bool                        sign        = false;                // Specifies whether or not this is a signed type (only for OpTypeInt).
            std::vector<const SpvType*> fieldTypes;                         // List of types of each record field.
        };

        // SPIRV-V scalar constants.
        struct SpvConstant
        {
            const SpvType*      type    = nullptr;
            union
            {
                float           f32;
                double          f64;
                std::uint32_t   u32;
                std::uint64_t   u64;
                std::int32_t    i32;
                std::int64_t    i64;
            };
        };

        // Global uniform objects.
        struct SpvUniform
        {
            const char*     name    = nullptr;
            const SpvType*  type    = nullptr;
            std::uint32_t   set     = 0;        // Descriptor set
            std::uint32_t   binding = 0;        // Binding point
            std::uint32_t   size    = 0;        // Size (in bytes) of the uniform.
        };

        // Module varyings, i.e. either input or output attributes.
        struct SpvVarying
        {
            const char*     name        = nullptr;
            spv::BuiltIn    builtin     = spv::BuiltInMax;  // Optional built-in type
            const SpvType*  type        = nullptr;
            std::uint32_t   location    = 0;
            bool            input       = false;
        };

        // Block field for a single push constant field.
        struct SpvBlockField
        {
            const char*     name    = nullptr;
            std::uint32_t   offset  = 0;
        };

        // Block reflection for push constants.
        struct SpvBlock
        {
            const char*                 name    = nullptr;
            std::vector<SpvBlockField>  fields;
        };

        // Binding point including descriptor set.
        struct SpvBindingPoint
        {
            spv::Id         id                  = 0; // Resource ID.
            std::uint32_t   set                 = 0; // Descriptor set
            std::uint32_t   setWordOffset       = 0; // Word offset within the SPIR-V module of the descriptor set.
            std::uint32_t   binding             = 0; // Binding point
            std::uint32_t   bindingWordOffset   = 0; // Word offset within the SPIR-V module of the binding point.
        };

    public:

        // Parse all instructions in the specified SPIR-V module.
        SpirvResult Reflect(const SpirvModuleView& module);

    public:

        // Returns the container that maps a SPIR-V ID to its type definition.
        inline const std::map<spv::Id, SpvType>& GetTypes() const
        {
            return types_;
        }

        // Returns the container that maps a SPIR-V ID to its constant definition.
        inline const std::map<spv::Id, SpvConstant>& GetConstants() const
        {
            return constants_;
        }

        // Returns the container that maps a SPIR-V ID to its uniform definition.
        inline const std::map<spv::Id, SpvUniform>& GetUniforms() const
        {
            return uniforms_;
        }

        // Returns the container that maps a SPIR-V ID to its varying definition.
        inline const std::map<spv::Id, SpvVarying>& GetVaryings() const
        {
            return varyings_;
        }

    private:

        using Instr = SpirvInstruction;

        SpirvResult ParseInstruction(const SpirvInstruction& instr);

        SpirvResult OpName(const Instr& instr);

        SpirvResult OpDecorate(const Instr& instr);
        void OpDecorateBinding(const Instr& instr, spv::Id id);
        void OpDecorateLocation(const Instr& instr, spv::Id id);
        void OpDecorateBuiltin(const Instr& instr, spv::Id id);

        SpirvResult OpVariable(const Instr& instr);
        SpirvResult OpConstant(const Instr& instr);

        SpirvResult OpType(const Instr& instr);
        void OpTypeVoid(const Instr& instr, SpvType& type);
        void OpTypeBool(const Instr& instr, SpvType& type);
        void OpTypeInt(const Instr& instr, SpvType& type);
        void OpTypeFloat(const Instr& instr, SpvType& type);
        void OpTypeVector(const Instr& instr, SpvType& type);
        void OpTypeMatrix(const Instr& instr, SpvType& type);
        void OpTypeImage(const Instr& instr, SpvType& type);
        void OpTypeSampler(const Instr& instr, SpvType& type);
        void OpTypeSampledImage(const Instr& instr, SpvType& type);
        void OpTypeArray(const Instr& instr, SpvType& type);
        void OpTypeRuntimeArray(const Instr& instr, SpvType& type);
        void OpTypeStruct(const Instr& instr, SpvType& type);
        void OpTypeOpaque(const Instr& instr, SpvType& type);
        void OpTypePointer(const Instr& instr, SpvType& type);
        void OpTypeFunction(const Instr& instr, SpvType& type);

        const SpvType* FindType(spv::Id id) const;
        const SpvConstant* FindConstant(spv::Id id) const;

    private:

        std::uint32_t                   idBound_    = 0;
        SpirvNameDecorations            names_;

        std::map<spv::Id, SpvType>      types_;
        std::map<spv::Id, SpvConstant>  constants_;
        std::map<spv::Id, SpvUniform>   uniforms_;
        std::map<spv::Id, SpvVarying>   varyings_;

};


// Reflect the specified SPIR-V module only for the execution mode.
SpirvResult SpirvReflectExecutionMode(const SpirvModuleView& module, SpirvReflect::SpvExecutionMode& outExecutionMode);

// Reflect the specified SPIR-V module only for push constants.
SpirvResult SpirvReflectPushConstants(const SpirvModuleView& module, SpirvReflect::SpvBlock& outBlock);

// Reflect the specified SPIR-V module only for binding points (including their descriptor sets).
SpirvResult SpirvReflectBindingPoints(const SpirvModuleView& module, std::vector<SpirvReflect::SpvBindingPoint>& outBindingPoints);


} // /namespace LLGL


#endif



// ================================================================================
