xof 0302txt 0064
template Header {
 <3D82AB43-62DA-11cf-AB39-0020AF71E433>
 WORD major;
 WORD minor;
 DWORD flags;
}

template Vector {
 <3D82AB5E-62DA-11cf-AB39-0020AF71E433>
 FLOAT x;
 FLOAT y;
 FLOAT z;
}

template Coords2d {
 <F6F23F44-7686-11cf-8F52-0040333594A3>
 FLOAT u;
 FLOAT v;
}

template Matrix4x4 {
 <F6F23F45-7686-11cf-8F52-0040333594A3>
 array FLOAT matrix[16];
}

template ColorRGBA {
 <35FF44E0-6C7C-11cf-8F52-0040333594A3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
 FLOAT alpha;
}

template ColorRGB {
 <D3E16E81-7835-11cf-8F52-0040333594A3>
 FLOAT red;
 FLOAT green;
 FLOAT blue;
}

template IndexedColor {
 <1630B820-7842-11cf-8F52-0040333594A3>
 DWORD index;
 ColorRGBA indexColor;
}

template Boolean {
 <4885AE61-78E8-11cf-8F52-0040333594A3>
 WORD truefalse;
}

template Boolean2d {
 <4885AE63-78E8-11cf-8F52-0040333594A3>
 Boolean u;
 Boolean v;
}

template MaterialWrap {
 <4885AE60-78E8-11cf-8F52-0040333594A3>
 Boolean u;
 Boolean v;
}

template TextureFilename {
 <A42790E1-7810-11cf-8F52-0040333594A3>
 STRING filename;
}

template Material {
 <3D82AB4D-62DA-11cf-AB39-0020AF71E433>
 ColorRGBA faceColor;
 FLOAT power;
 ColorRGB specularColor;
 ColorRGB emissiveColor;
 [...]
}

template MeshFace {
 <3D82AB5F-62DA-11cf-AB39-0020AF71E433>
 DWORD nFaceVertexIndices;
 array DWORD faceVertexIndices[nFaceVertexIndices];
}

template MeshFaceWraps {
 <4885AE62-78E8-11cf-8F52-0040333594A3>
 DWORD nFaceWrapValues;
 Boolean2d faceWrapValues;
}

template MeshTextureCoords {
 <F6F23F40-7686-11cf-8F52-0040333594A3>
 DWORD nTextureCoords;
 array Coords2d textureCoords[nTextureCoords];
}

template MeshMaterialList {
 <F6F23F42-7686-11cf-8F52-0040333594A3>
 DWORD nMaterials;
 DWORD nFaceIndexes;
 array DWORD faceIndexes[nFaceIndexes];
 [Material]
}

template MeshNormals {
 <F6F23F43-7686-11cf-8F52-0040333594A3>
 DWORD nNormals;
 array Vector normals[nNormals];
 DWORD nFaceNormals;
 array MeshFace faceNormals[nFaceNormals];
}

template MeshVertexColors {
 <1630B821-7842-11cf-8F52-0040333594A3>
 DWORD nVertexColors;
 array IndexedColor vertexColors[nVertexColors];
}

template Mesh {
 <3D82AB44-62DA-11cf-AB39-0020AF71E433>
 DWORD nVertices;
 array Vector vertices[nVertices];
 DWORD nFaces;
 array MeshFace faces[nFaces];
 [...]
}

Header{
1;
0;
1;
}

Mesh {
 19;
 10.00000;-0.00000;-0.37500;,
 10.00000;-0.26517;-0.26517;,
 0.00000;-2.65165;-2.65165;,
 0.00000;0.00000;-3.75000;,
 10.00000;-0.37500;0.00000;,
 0.00000;-3.75000;0.00000;,
 10.00000;-0.26517;0.26517;,
 0.00000;-2.65165;2.65165;,
 10.00000;-0.00000;0.37500;,
 0.00000;0.00000;3.75000;,
 10.00000;0.26516;0.26516;,
 0.00000;2.65165;2.65165;,
 10.00000;0.37500;-0.00000;,
 0.00000;3.75000;-0.00000;,
 10.00000;0.26516;-0.26517;,
 0.00000;2.65165;-2.65165;,
 0.00000;2.65165;-2.65165;,
 10.00000;-0.00000;-0.00000;,
 0.00000;0.00000;0.00000;;
 
 24;
 4;0,1,2,3;,
 4;1,4,5,2;,
 4;4,6,7,5;,
 4;6,8,9,7;,
 4;8,10,11,9;,
 4;10,12,13,11;,
 4;12,14,15,13;,
 4;14,0,3,16;,
 3;17,1,0;,
 3;17,4,1;,
 3;17,6,4;,
 3;17,8,6;,
 3;17,10,8;,
 3;17,12,10;,
 3;17,14,12;,
 3;17,0,14;,
 3;18,3,2;,
 3;18,2,5;,
 3;18,5,7;,
 3;18,7,9;,
 3;18,9,11;,
 3;18,11,13;,
 3;18,13,16;,
 3;18,16,3;;
 
 MeshMaterialList {
  1;
  24;
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0;;
  Material {
   1.000000;1.000000;1.000000;1.000000;;
   5.000000;
   0.000000;0.000000;0.000000;;
   0.000000;0.000000;0.000000;;
   TextureFilename {
    "data\\TEXTURE\\drill000.jpg";
   }
  }
 }
 MeshNormals {
  10;
  1.000000;0.000000;0.000000;,
  0.319779;0.000000;-0.947492;,
  0.319779;-0.669978;-0.669978;,
  0.319779;-0.947492;0.000000;,
  0.319779;-0.669978;0.669978;,
  0.319779;0.000000;0.947492;,
  0.319779;0.669978;0.669978;,
  0.319779;0.947492;-0.000000;,
  0.319779;0.669978;-0.669978;,
  -1.000000;0.000000;0.000000;;
  24;
  4;1,2,2,1;,
  4;2,3,3,2;,
  4;3,4,4,3;,
  4;4,5,5,4;,
  4;5,6,6,5;,
  4;6,7,7,6;,
  4;7,8,8,7;,
  4;8,1,1,8;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;0,0,0;,
  3;9,9,9;,
  3;9,9,9;,
  3;9,9,9;,
  3;9,9,9;,
  3;9,9,9;,
  3;9,9,9;,
  3;9,9,9;,
  3;9,9,9;;
 }
 MeshTextureCoords {
  19;
  0.728461;1.999360;,
  0.709251;1.998056;,
  0.848278;0.996181;,
  0.974273;1.009226;,
  0.670793;1.996944;,
  0.727612;0.985058;,
  0.627420;1.996675;,
  0.609517;0.982372;,
  0.585461;1.997408;,
  0.490591;0.989697;,
  0.553284;1.998712;,
  0.367599;1.002742;,
  0.558545;1.999824;,
  0.238704;1.013865;,
  0.671750;2.000093;,
  0.105782;1.016551;,
  1.105782;1.016551;,
  0.637980;1.998384;,
  0.637980;0.999461;;
 }
}
