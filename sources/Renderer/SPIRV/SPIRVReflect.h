/*
 * SPIRVReflect.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SPIRV_REFLECT_H
#define LLGL_SPIRV_REFLECT_H


#include "SPIRVParser.h"
#include <vector>
#include <map>


namespace LLGL
{


// SPIR-V shader module parser.
class SPIRVReflect final : public SPIRVParser
{

    public:

        // General purpose structure for all SPIR-V module types.
        struct SpvType
        {
            const SpvType* DereferencePtr() const;
            const SpvType* DereferencePtr(const spv::Op opcodeType) const;
            bool RefersToType(const spv::Op opcodeType) const;

            spv::Op                     opcode      = spv::Op::Max;             // Opcode for this type (e.g. spv::Op::OpTypeFloat).
            spv::Id                     result      = 0;                        // Result ID of this type.
            spv::StorageClass           storage     = spv::StorageClass::Max;   // Storage class of this type. By default spv::StorageClass::Max.
            const char*                 name        = nullptr;                  // Name of this type (only for structures).
            const SpvType*              baseType    = nullptr;                  // Reference to the base type, or null if there is no base type.
            std::uint32_t               elements    = 0;                        // Number of elements for the base type, or 0 if there is no base type.
            std::uint32_t               size        = 0;                        // Size (in bytes) of this type, or 0 if this is an OpTypeVoid type.
            bool                        sign        = false;                    // Specifies whether or not this is a signed type (only for OpTypeInt).
            std::vector<const SpvType*> fieldTypes;                             // List of types of each record field.
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

        // SPIR-V structures (a.k.a. records).
        struct SpvRecord
        {
            const char*     name    = nullptr;
            std::uint32_t   size    = 0;
            std::uint32_t   padding = 0;
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
            spv::BuiltIn    builtin     = spv::BuiltIn::Max;    // Optional built-in type
            const SpvType*  type        = nullptr;
            std::uint32_t   location    = 0;
            bool            input       = false;
        };

    public:

        inline const std::map<spv::Id, SpvRecord>& GetRecords() const
        {
            return records_;
        }

        inline const std::map<spv::Id, SpvUniform>& GetUniforms() const
        {
            return uniforms_;
        }

        inline const std::map<spv::Id, SpvVarying>& GetVaryings() const
        {
            return varyings_;
        }

    private:

        using Instr = SPIRVInstruction;

        void OnParseHeader(const SPIRVHeader& header) override;
        void OnParseInstruction(const SPIRVInstruction& instr) override;

        void OpName(const Instr& instr);
        void OpDecorate(const Instr& instr);
        void OpDecorateBinding(const Instr& instr);
        void OpDecorateLocation(const Instr& instr);
        void OpDecorateBuiltin(const Instr& instr);
        void OpType(const Instr& instr);
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
        void OpVariable(const Instr& instr);
        void OpConstant(const Instr& instr);

    private:

        void SetName(spv::Id id, const char* name);
        const char* GetName(spv::Id id) const;

        void AssertIdBound(spv::Id id) const;

        const SpvType* FindType(spv::Id id) const;
        const SpvConstant* FindConstant(spv::Id id) const;

    private:

        std::uint32_t                   idBound_    = 0;
        std::vector<const char*>        names_;

        std::map<spv::Id, SpvType>      types_;
        std::map<spv::Id, SpvConstant>  constants_;
        std::map<spv::Id, SpvRecord>    records_;
        std::map<spv::Id, SpvUniform>   uniforms_;
        std::map<spv::Id, SpvVarying>   varyings_;

};


} // /namespace LLGL


#endif



// ================================================================================
