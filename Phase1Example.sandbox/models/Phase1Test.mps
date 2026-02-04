<?xml version="1.0" encoding="UTF-8"?>
<model ref="r:c1d2e3f4-a5b6-47c8-9d0e-f1a2b3c4d5e6(Phase1Test)">
  <persistence version="9" />
  <languages>
    <use id="a452e1cf-6f8c-4bca-8447-fd4d6e4d0003" name="SemAnno" version="0" />
    <use id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core" version="2" />
  </languages>
  <imports />
  <registry>
    <language id="a452e1cf-6f8c-4bca-8447-fd4d6e4d0003" name="SemAnno">
      <concept id="8440420766104857128" name="SemAnno.structure.Module" flags="ng" index="2Ks9RF">
        <child id="8982541288447632589" name="variables" index="2Ks9RG" />
        <child id="8982541288447632588" name="functions" index="2Ks9RH" />
      </concept>
      <concept id="8982541288447632700" name="SemAnno.structure.Variable" flags="ng" index="2KtM7O" />
      <concept id="8440420766104857192" name="SemAnno.structure.Function" flags="ng" index="2Ks9S5" />
    </language>
    <language id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core">
      <concept id="1169194658468" name="jetbrains.mps.lang.core.structure.INamedConcept" flags="ngI" index="TrEIO">
        <property id="1169194664001" name="name" index="TrG5h" />
      </concept>
    </language>
  </registry>
  <node concept="2Ks9RF" id="Phase1_00001">
    <property role="TrG5h" value="TestModule" />
    <node concept="2KtM7O" id="Phase1_00002" role="2Ks9RG">
      <property role="TrG5h" value="testVar" />
    </node>
  </node>
</model>
