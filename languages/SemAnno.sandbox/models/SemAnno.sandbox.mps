<?xml version="1.0" encoding="UTF-8"?>
<model ref="r:7e00567c-a09a-4d4d-bc4a-644a7db85ea4(SemAnno.sandbox)">
  <persistence version="9" />
  <languages>
    <use id="a452e1cf-6f8c-4bca-8447-fd4d6e4d0003" name="SemAnno" version="0" />
  </languages>
  <imports />
  <registry>
    <language id="a452e1cf-6f8c-4bca-8447-fd4d6e4d0003" name="SemAnno">
      <concept id="725769291168392482" name="SemAnno.structure.SemanticEntity" flags="ng" index="YUdpL">
        <child id="725769291168392486" name="components" index="YUdpP" />
      </concept>
      <concept id="725769291168515475" name="SemAnno.structure.StringList" flags="ng" index="YVJr0">
        <property id="725769291168515476" name="values" index="YVJr7" />
      </concept>
      <concept id="725769291168515491" name="SemAnno.structure.Iterator" flags="ng" index="YVJrK">
        <reference id="725769291168515492" name="target" index="YVJrR" />
      </concept>
      <concept id="725769291168885093" name="SemAnno.structure.Script" flags="ng" index="Z45CQ">
        <child id="725769291168885096" name="entities" index="Z45CV" />
      </concept>
    </language>
    <language id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core">
      <concept id="1169194658468" name="jetbrains.mps.lang.core.structure.INamedConcept" flags="ngI" index="TrEIO">
        <property id="1169194664001" name="name" index="TrG5h" />
      </concept>
    </language>
  </registry>
  <node concept="Z45CQ" id="CisPcP0I_m">
    <property role="TrG5h" value="RunMe" />
    <node concept="YUdpL" id="CisPcP0Qn6" role="Z45CV">
      <property role="TrG5h" value=" MyData" />
      <node concept="YVJr0" id="CisPcP0Qn7" role="YUdpP">
        <property role="YVJr7" value="A, B, C" />
      </node>
    </node>
    <node concept="YUdpL" id="CisPcP0Qnb" role="Z45CV">
      <property role="TrG5h" value="TheLoop" />
      <node concept="YVJrK" id="CisPcP0Qnc" role="YUdpP">
        <ref role="YVJrR" node="CisPcP0Qnb" resolve="TheLoop" />
      </node>
    </node>
  </node>
</model>

