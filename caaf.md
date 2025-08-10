# Compact Actor Archive Format

Current version: 0  
Based on SDL 3.2.x
  
All numbers are in base 16.  

## General definitions

A **Compact Actor Archive Format** or **CAAF** for short is a binary archive format which is used to store all data required to render a 3D model in SDL3 using the GPU API as well as other optional data. This is **not** an official format.  
  
Each CAAF contains a header and a set of sections.

### Sections

Each section is based on the following definition:

| Offset | Size | Sign | Name    | Description                                              |
| ------ | ---- | ---- | ------- | -------------------------------------------------------- |
| 00     | 04   | -    | Magic   | Magic in ASCII                                           |
| 04     | 04   | No   | Size    | Amount of memory in bytes that the section takes.        |
| 08     | 04   | No   | Count   | Amount of entries in the section.                        |

Section data is akin to C-style arrays (`void**`). Each section has an array of 4-byte unsigned integers after its header. Every integer is a relative pointer (relative to itself) to an entry with a format that matches the section's definition.

### Subsections

A subsection header is also defined, where the data type is implicit and of constant size:

| Offset | Size | Sign | Name    | Description                                              |
| ------ | ---- | ---- | ------- | -------------------------------------------------------- |
| 00     | 02   | No   | Count   | Amount of entries in the subsection.                     |
| 02     | 02   | No   | Size    | The size in bytes of each entry in the subsection.       |

All data in the subsection is stored inlined, such as instead of storing pointers towards the data, data is stored directly in the array.

### Dependencies

A CAAF may optionally depend on another file whenever data may be repeated throughout multiple actors. For example, when the same textures are used repeatedly.  
When parsing a CAAF file that has a dependency, the contents of the dependency are considered to be appended at the end of each relevant section. So any index references must take this in mind.

### Other definitions

Sections, subsections and section entry data should be aligned to 16 bytes, although it is not required.  
Section sizes do not include the section header.  
Pointers within entries of sections and subsections are always relative to the start of that entry.

## Header

Header of the CAAF format, which is always present at the start of the file.

| Offset | Size | Sign | Name    | Description                                              |
| ------ | ---- | ---- | ------- | -------------------------------------------------------- |
| 00     | 04   | -    | Magic   | Magic in ASCII: CAAF                                     |
| 04     | 01   | No   | Version | Currently 0.                                             |
| 05     | 01   | No   | IsDep   | If not set to 0, this file is treated as a dependency.   |
| 06     | 02   | No   | SectCnt | Section count, amount of sections in the archive.        |
| 08     | 02   | No   | Name    | Index of a string indicating the actor's name.           |
| 0A     | 02   | No   | Depends | Index of a string indicating a dependency of the actor.  |

Whenever IsDep is set, then the current file is meant to be read as a dependency of other actors.

## Section list

Always located starting byte 10 in the file.  
The first section is always the string table, other sections are optional and can appear in any order.  
For each section, a 32-bit unsigned integer absolute pointer is stored.

## String Table section

Magic: STRT  
This section stores all of the strings used throughout the archive. Every entry in the array points to a null-terminated string.  
The first entry is always an empty string ("").

## Mesh section

Magic: MESH  
Contains vertex and index information for each mesh.  
Entry contents are as follows:

| Offset | Size | Sign | Name    | Description                                              |
| ------ | ---- | ---- | ------- | -------------------------------------------------------- |
| 00     | 04   | No   | VBDPtr  | Pointer to vertex buffer data subsection.                |
| 04     | 04   | No   | VertPtr | Pointer to mesh vertex data.                             |
| 08     | 04   | No   | IdxCnt  | Amount of indices in the mesh.                           |
| 0C     | 04   | No   | IdxPtr  | Pointer to mesh index data.                              |

VertPtr refers to a data block of undefined size that is meant to be read in parts as per what is defined in the vertex buffer data subsection.  
IdxPtr refers to a data block of size IdxCnt * 4 containing elements of 4 bytes each.

### Vertex Buffer Data subsection

Size of data: 08  
Each vertex buffer data entry refers to a vertex buffer description entry of the same index and is defined as follows:

| Offset | Size | Sign | Name    | Description                                              |
| ------ | ---- | ---- | ------- | -------------------------------------------------------- |
| 00     | 04   | No   | Start   | The start of the buffer, relative to mesh data start.    |
| 04     | 04   | No   | Length  | The lenght of the buffer.                                |

## Graphics Pipeline section

Magic: GFXP  
Each graphics pipeline entry refers to the mesh with the same index. For example, the first graphics pipeline entry refers to the first mesh and so on.  
Entry contents are as follows:

| Offset | Size | Sign | Name    | Description                                              |
| ------ | ---- | ---- | ------- | -------------------------------------------------------- |
| 00     | 02   | No   | VertNm  | Index of a string indicating the vertex shader name.     |
| 02     | 02   | No   | FragNm  | Index of a string indicating the fragment shader name.   |
| 04     | 01   | No   | PType   | The primitive type to use when rendering.                |
| 05     | 01   | No   | FillMod | Describes how polygons are filled.                       |
| 06     | 01   | No   | CullMod | The facing direction in which triangles will be culled.  |
| 07     | 01   | No   | FrtFace | Specifies the vertex winding to determine front face.    |
| 08     | 01   | No   | MSAACnt | The number of samples to be used in rasterization.       |
| 09     | 01   | No   | CompOp  | The comparison operator used for depth testing.          |
| 0A     | 01   | No   | BackOp  | The stencil op state for back-facing triangles.          |
| 0B     | 01   | No   | FrontOp | The stencil op state for front-facing triangles.         |
| 0C     | 01   | No   | CmpMask | The compare mask used in stencil test.                   |
| 0D     | 01   | No   | WrtMask | The write mask used in stencil test.                     |
| 0E     | 02   | No   | EnFlags | Various flags to enable or disable parameters.           |
| 10     | 04   | Yes* | DBConst | Factor of the depth value added to each fragment.        |
| 14     | 04   | Yes* | DBClamp | The maximum depth bias of each fragment.                 |
| 18     | 04   | Yes* | DBSlope | Factor applied to the fragment's slope in depth calc.    |
| 1C     | 04   | No   | Props   | A properties ID for extensions. 0 if none used.          |
| 20     | 04   | No   | VBDPtr  | Pointer to vertex buffer description subsection.         |
| 24     | 04   | No   | VAPtr   | Pointer to vertex attribute subsection.                  |
| 28     | 04   | No   | CTBPtr  | Pointer to color target blending subsection.             |
| 2C     | 04   | No   | TSBPtr  | Pointer to texture sampler binding subsection.           |

The values of PType represent the values in `SDL_GPUPrimitiveType`.  
The values of FillMod represent the values in `SDL_GPUFillMode`.  
The values of CullMod represent the values in `SDL_GPUCullMode`.  
The values of FrtFace represent the values in `SDL_GPUFrontFace`.  
The values of MSAACnt represent the values in `SDL_GPUSampleCount`.  
The values of CompOp represent the values in `SDL_GPUCompareOp`.  
The values of BackOp and FrontOp represent the values in `SDL_GPUStencilOpState`.  
\* DBConst, DBClamp and DBSlope are floats.  
  
The following flags are available in EnFlags:

| Bit | Name    | Description                                              |
| --- | ------- | -------------------------------------------------------- |
| 0   | EnDBias | Enables depth bias.                                      |
| 1   | EnDClip | Enables depth clip, otherwise use depth clamp.           |
| 2   | EnDTest | Enables depth test.                                      |
| 3   | EnDWrt  | Enables depth write.                                     |
| 4   | EnDStcl | Enables depth stencil. Also affects target info.         |
| 5   | EnA2Cov | Enables alpha to coverage.                               |
| 6   | EnMask  | Reserved for future use.                                 |

Flags are considered enabled when the bit is set to 1.

### Vertex Buffer Description subsection

Size of data: 0C  
Each vertex buffer description entry is defined as follows:

| Offset | Size | Sign | Name    | Description                                              |
| ------ | ---- | ---- | ------- | -------------------------------------------------------- |
| 00     | 04   | No   | Slot    | The binding slot of the vertex buffer.                   |
| 04     | 04   | No   | Pitch   | The byte pitch between consecutive elements.             |
| 08     | 04   | No   | InstStp | Instance step rate. Currently restricted to 0 or 1.      |

InstStp is meant to be used in the future when the GPU API supports instance step rate. For now, it is only used to define `SDL_GPUVertexBufferDescription::input_rate`: 0 refers to `SDL_GPU_VERTEXINPUTRATE_VERTEX` and 1 refers to `SDL_GPU_VERTEXINPUTRATE_INSTANCE`.

### Vertex Attribute subsection

Size of data: 10  
Each vertex attribute entry is defined as follows:

| Offset | Size | Sign | Name    | Description                                              |
| ------ | ---- | ---- | ------- | -------------------------------------------------------- |
| 00     | 02   | No   | Loc     | The location in the shader.                              |
| 02     | 02   | No   | Format  | The type of the attribute data.                          |
| 04     | 04   | No   | Slot    | The binding slot of the associated vertex buffer.        |
| 0C     | 04   | No   | Offset  | The offset in bytes relative to the start of the vertex. |

The values of Format represent the values in `SDL_GPUVertexElementFormat`.

### Color Target Blending subsection

Stores data to be used when setting the blend state of a color target. The color targets themselves are not defined within this file so the use of this subsection depends on how the parsing is handled. If the subsection is empty, it is assumed that no blending is required.  
  
Size of data: 08  
Each color target blending entry is defined as follows:

| Offset | Size | Sign | Name    | Description                                              |
| ------ | ---- | ---- | ------- | -------------------------------------------------------- |
| 00     | 01   | No   | SrcCol  | The factor to be multiplied to the source color.         |
| 01     | 01   | No   | DstCol  | The factor to be multiplied to the destination color.    |
| 02     | 01   | No   | ColBOp  | The color blend operation.                               |
| 03     | 01   | No   | SrcAlph | The factor to be multiplied to the source alpha.         |
| 04     | 01   | No   | DstAlph | The factor to be multiplied to the destination alpha.    |
| 05     | 01   | No   | AlphBOp | The alpha blend operation.                               |
| 06     | 01   | No   | RGBAMsk | An RGBA mask that defines which components to write.     |
| 07     | 01   | No   | EnFlags | Various flags to enable or disable parameters.           |

The values of SrcCol, DstCol, SrcAlph and DstAlph represent the values in `SDL_GPUBlendFactor`.  
The values of ColBOp and AlphBOp represent the values in `SDL_GPUBlendOp`.  
The values of RGBAMsk are based on the defines on `SDL_GPUColorComponentFlags`.  
  
The following flags are available in EnFlags:

| Bit | Name    | Description                                              |
| --- | ------- | -------------------------------------------------------- |
| 0   | EnBlend | Enables blending for the color target.                   |
| 1   | EnMask  | Enables the RGBA mask for the color target.              |

Flags are considered enabled when the bit is set to 1.

### Texture Sampler Binding subsection

Size of data: 0C  
Each texture sampler binding entry is defined as follows:

| Offset | Size | Sign | Name    | Description                                              |
| ------ | ---- | ---- | ------- | -------------------------------------------------------- |
| 00     | 04   | No   | Slot    | The shader slot the texture sampler has to be bound to.  |
| 04     | 02   | No   | TextIdx | The index of the texture to bind.                        |
| 06     | 02   | No   | SampIdx | The index of the sampler to use with the texture.        |
| 08     | 01   | No   | ShStage | The shader stage the texture sampler has to be bound to. |

The values of ShStage represent the values in `SDL_GPUShaderStage`.

## Texture section

Magic: TEXD  
Stores texture definitions and their data.  
Entry contents are as follows:

| Offset | Size | Sign | Name    | Description                                              |
| ------ | ---- | ---- | ------- | -------------------------------------------------------- |
| 00     | 01   | No   | Type    | The texture type (dimension, array, etc).                |
| 01     | 01   | No   | Format  | The format of the pixels in the texture.                 |
| 02     | 02   | No   | MipLvls | The number of mip levels in the texture.                 |
| 04     | 04   | No   | Width   | The width in pixels of the texture.                      |
| 08     | 04   | No   | Height  | The height in pixels of the texture.                     |
| 0C     | 04   | No   | Depth   | The depth of the texture if in 3D, else layer count.     |
| 10     | 04   | No   | Props   | A properties ID for extensions. 0 if none used.          |
| 14     | 04   | No   | DataPtr | Pointer to texture data.                                 |

The values of Type represent the values in `SDL_GPUTextureType`.  
The values of Format represent the values in `SDL_GPUTextureFormat`.  
MipLvls is used to define the amount of mipmaps that will be automatically generated.  
Size of texture data is obtained by multiplying width, height and depth. All of them must not be 0.

## Sampler section

Magic: SAMP
Stores sample definitions which can be used by more than one texture.  
Entry contents are as follows:

| Offset | Size | Sign | Name    | Description                                              |
| ------ | ---- | ---- | ------- | -------------------------------------------------------- |
| 00     | 01   | No   | MinFilt | The filter to apply on minification.                     |
| 01     | 01   | No   | MagFilt | The filter to apply on magnification.                    |
| 02     | 01   | No   | MapMode | The mipmap filter.                                       |
| 03     | 01   | No   | AddrMdU | The address mode for U coordinates.                      |
| 04     | 01   | No   | AddrMdV | The address mode for V coordinates.                      |
| 05     | 01   | No   | AddrMdW | The address mode for W coordinates.                      |
| 06     | 01   | No   | CompOp  | The comparison operator to apply to fetched data.        |
| 07     | 01   | No   | EnFlags | Various flags to enable or disable parameters.           |
| 08     | 04   | Yes* | MipLODB | The bias to be added to mipmap LOD calculation.          |
| 0C     | 04   | Yes* | MaxAnis | The maximum anisotropy value.                            |
| 10     | 04   | Yes* | MinLOD  | The minimum LOD value.                                   |
| 14     | 04   | Yes* | MaxLOD  | The maximum LOD value.                                   |
| 18     | 04   | No   | Props   | A properties ID for extensions. 0 if none used.          |

The values of MinFilt and MagFilt represent the values in `SDL_GPUFilter`.  
The values of MapMode represent the values in `SDL_GPUSamplerMipmapMode`.  
The values of AddrMdU, AddrMdV and AddrMdW represent the values in `SDL_GPUSamplerAddressMode`.  
The values of CompOp represent the values in `SDL_GPUCompareOp`.  
\* MipLODB, MaxAnis, MixLOD and MaxLOD are floats.  
  
The following flags are available in EnFlags:

| Bit | Name    | Description                                              |
| --- | ------- | -------------------------------------------------------- |
| 0   | EnAnis  | Enables anisotropic filtering.                           |
| 1   | EnComp  | Enables comparison against a reference value.            |
