# Compact Actor Archive Format

Current version: 0  
Based on SDL 3.2.x  
  
All numbers are in base 16.  

## General definition

A **Compact Shader Archive Format** or **CSAF** for short is a binary archive format which is used to store a precompiled shader in one or more formats.  
It is meant to be used alongside [**CAAF**](caaf.md).

## Header

Header of the CSAF format, which is always present at the start of the file.

| Offset | Size | Sign | Name    | Description                                              |
| ------ | ---- | ---- | ------- | -------------------------------------------------------- |
| 00     | 04   | -    | Magic   | Magic in ASCII: CSAF                                     |
| 04     | 01   | No   | Version | Currently 0.                                             |
| 05     | 01   | No   | Stage   | The stage of the shader: vertex, fragment, etc.          |
| 06     | 01   | No   | SampCnt | The number of samplers defined in the shader.            |
| 07     | 01   | No   | StoTCnt | The number of storage textures defined in the shader.    |
| 08     | 01   | No   | StoBCnt | The number of storage buffers defined in the shader.     |
| 09     | 01   | No   | UniBCnt | The number of uniform buffers defined in the shader.     |
| 0A     | 02   | No   | ShForms | The formats of the shader: SPIR-V, DXIL, etc.            |
| 0C     | 04   | No   | Props   | A properties ID for extensions. 0 if none used.          |

The values of Stage represent the values of ``SDL_GPUShaderStage``.  
The values of ShForms are based on the defines on ``SDL_GPUShaderFormat``.

## Shader list

Always located starting byte 10 in the file.  
The list is sorted based on the values in ShForms. For example, SPIR-V (bit 1) will come before DXIL (bit 3). Check ``SDL_GPUShaderFormat`` for more details.  
For each shader format there is an entry defined as:

| Offset | Size | Sign | Name    | Description                                              |
| ------ | ---- | ---- | ------- | -------------------------------------------------------- |
| 00     | 04   | No   | Size    | Size of the shader in bytes.                             |
| 04     | 04   | No   | Pointer | Pointer to the shader's first byte.                      |

Entries are stored one after the other in an array.

## Shaders

Shaders are stored as binary data blobs and are accessed through the shader list.
