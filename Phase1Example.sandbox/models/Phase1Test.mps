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
      <concept id="8982541288447632700" name="SemAnno.structure.Variable" flags="ng" index="2EAK29">
        <child id="8982541288447632702" name="type" index="2EAK2b" />
      </concept>
      <concept id="8982541288447632652" name="SemAnno.structure.VariableReference" flags="ng" index="2EAK2T">
        <property id="8982541288447632653" name="variableName" index="2EAK2S" />
      </concept>
      <concept id="8982541288447632557" name="SemAnno.structure.BinaryOperation" flags="ng" index="2EAK4o">
        <property id="8982541288447632558" name="operator" index="2EAK4r" />
        <child id="8982541288447632560" name="right" index="2EAK45" />
        <child id="8982541288447632559" name="left" index="2EAK4q" />
      </concept>
      <concept id="8982541288447632492" name="SemAnno.structure.Return" flags="ng" index="2EAK7p">
        <child id="8982541288447632493" name="value" index="2EAK7o" />
      </concept>
      <concept id="8440420766104844840" name="SemAnno.structure.PrimitiveType" flags="ng" index="3rC1Zv">
        <property id="8982541288447632334" name="kind" index="2EAKpV" />
      </concept>
      <concept id="8440420766104857256" name="SemAnno.structure.Parameter" flags="ng" index="3rCcXv">
        <child id="8982541288447632653" name="type" index="2EAK2S" />
      </concept>
      <concept id="8440420766104857192" name="SemAnno.structure.Function" flags="ng" index="3rCcYv">
        <child id="8982541288447632621" name="returnType" index="2EAK5o" />
        <child id="8982541288447632623" name="parameters" index="2EAK5q" />
        <child id="8982541288447632622" name="body" index="2EAK5r" />
      </concept>
      <concept id="8440420766104857128" name="SemAnno.structure.Module" flags="ng" index="3rCcZv">
        <property id="8982541288447632592" name="targetLanguage" index="2EAK5_" />
        <child id="8982541288447632589" name="functions" index="2EAK5S" />
        <child id="8982541288447632590" name="variables" index="2EAK5V" />
      </concept>
      <concept id="8440420766104853160" name="SemAnno.structure.Assignment" flags="ng" index="3rCfXv">
        <child id="8982541288447632365" name="target" index="2EAKpo" />
        <child id="8982541288447632366" name="value" index="2EAKpr" />
      </concept>
    </language>
    <language id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core">
      <concept id="1169194658468" name="jetbrains.mps.lang.core.structure.INamedConcept" flags="ngI" index="TrEIO">
        <property id="1169194664001" name="name" index="TrG5h" />
      </concept>
    </language>
  </registry>
  <node concept="3rCcZv" id="Calc_M001">
    <property role="TrG5h" value="Calculator" />
    <property role="2EAK5_" value="c" />
    <node concept="2EAK29" id="Calc_V001" role="2EAK5V">
      <property role="TrG5h" value="PI" />
      <node concept="3rC1Zv" id="Calc_VT001" role="2EAK2b">
        <property role="2EAKpV" value="float" />
      </node>
    </node>
    <node concept="3rCcYv" id="Calc_F001" role="2EAK5S">
      <property role="TrG5h" value="add" />
      <node concept="3rC1Zv" id="Calc_RT001" role="2EAK5o">
        <property role="2EAKpV" value="int" />
      </node>
      <node concept="3rCcXv" id="Calc_P001" role="2EAK5q">
        <property role="TrG5h" value="x" />
        <node concept="3rC1Zv" id="Calc_PT001" role="2EAK2S">
          <property role="2EAKpV" value="int" />
        </node>
      </node>
      <node concept="3rCcXv" id="Calc_P002" role="2EAK5q">
        <property role="TrG5h" value="y" />
        <node concept="3rC1Zv" id="Calc_PT002" role="2EAK2S">
          <property role="2EAKpV" value="int" />
        </node>
      </node>
      <node concept="3rCfXv" id="Calc_A001" role="2EAK5r">
        <property role="TrG5h" value="result" />
        <node concept="2EAK2T" id="c_VR_result" role="2EAKpo">
          <property role="2EAK2S" value="result" />
        </node>
        <node concept="2EAK4o" id="Calc_E001" role="2EAKpr">
          <property role="TrG5h" value="+" />
          <property role="2EAK4r" value="+" />
          <node concept="2EAK2T" id="Calc_VR_x" role="2EAK4q">
            <property role="2EAK2S" value="x" />
          </node>
          <node concept="2EAK2T" id="Calc_VR_y" role="2EAK45">
            <property role="2EAK2S" value="y" />
          </node>
        </node>
      </node>
      <node concept="2EAK7p" id="Calc_R001" role="2EAK5r">
        <node concept="2EAK2T" id="c_VR_return" role="2EAK7o">
          <property role="2EAK2S" value="result" />
        </node>
      </node>
    </node>
    <node concept="3rCcYv" id="Calc_F002" role="2EAK5S">
      <property role="TrG5h" value="multiply" />
      <node concept="3rC1Zv" id="Calc_RT002" role="2EAK5o">
        <property role="2EAKpV" value="int" />
      </node>
      <node concept="3rCcXv" id="Calc_P003" role="2EAK5q">
        <property role="TrG5h" value="a" />
        <node concept="3rC1Zv" id="Calc_PT003" role="2EAK2S">
          <property role="2EAKpV" value="int" />
        </node>
      </node>
      <node concept="3rCcXv" id="Calc_P004" role="2EAK5q">
        <property role="TrG5h" value="b" />
        <node concept="3rC1Zv" id="Calc_PT004" role="2EAK2S">
          <property role="2EAKpV" value="int" />
        </node>
      </node>
      <node concept="2EAK7p" id="Calc_R002" role="2EAK5r">
        <node concept="2EAK4o" id="Calc_E005" role="2EAK7o">
          <property role="TrG5h" value="*" />
          <property role="2EAK4r" value="*" />
          <node concept="2EAK2T" id="Calc_VR_a" role="2EAK4q">
            <property role="2EAK2S" value="a" />
          </node>
          <node concept="2EAK2T" id="Calc_VR_b" role="2EAK45">
            <property role="2EAK2S" value="b" />
          </node>
        </node>
      </node>
    </node>
  </node>
</model>

