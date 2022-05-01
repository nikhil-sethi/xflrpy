/****************************************************************************

    Shader locations struct
    Copyright (C) André Deperrois

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/

#pragma once


/** generic shader locations; not all locations are necessary nor defined for each shader */
struct ShaderLocations
{
    // Attribute data
    int m_attrVertex{-1}, m_attrNormal{-1};
    int m_attrColor{-1};
    int m_attrm_UV{-1};
    int m_attrUV{-1}; // vertex attribute array containing the texture's UV coordinates
    int m_attrOffset{-1};

    // Uniforms
    int m_vmMatrix{-1}, m_pvmMatrix{-1};
    int m_Scale{-1}; // only used if instancing is enabled

    int m_Light{-1};
    int m_UniColor{-1};
    int m_ClipPlane{-1};

    int m_TwoSided{-1};

    int m_HasUniColor{-1};
    int m_HasTexture{-1};    // uniform defining whether a texture is enabled or not
    int m_IsInstanced{-1};

    int m_Pattern{-1}, m_nPatterns{-1};
    int m_Thickness{-1}, m_Viewport{-1};

    int m_State{-1};
    int m_Shape{-1};

    int m_TexSampler{-1}; // the id of the sampler; defaults to 0
};
