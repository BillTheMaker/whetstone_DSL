<?xml version="1.0" encoding="UTF-8"?>
<model ref="r:a1b2c3d4-e5f6-47a8-b9c0-d1e2f3a4b5c6(SimpleFunctionExample)">
  <persistence version="9" />
  <languages>
    <use id="a452e1cf-6f8c-4bca-8447-fd4d6e4d0003" name="SemAnno" version="0" />
    <use id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core" version="2" />
  </languages>
  <imports />
  <registry>
    <language id="a452e1cf-6f8c-4bca-8447-fd4d6e4d0003" name="SemAnno">
      <concept id="8982541288447632652" name="SemAnno.structure.VariableReference" flags="ng" index="2EAK2T">
        <property id="8982541288447632654" name="variableName" index="2EAK2V" />
      </concept>
      <concept id="8982541288447632721" name="SemAnno.structure.DerefStrategy" flags="ng" index="2EAK3$">
        <property id="8982541288447632722" name="strategy" index="2EAK3B" />
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
        <child id="8982541288447632624" name="annotations" index="2EAK55" />
        <child id="8982541288447632621" name="returnType" index="2EAK5o" />
        <child id="8982541288447632623" name="parameters" index="2EAK5q" />
        <child id="8982541288447632622" name="body" index="2EAK5r" />
      </concept>
      <concept id="8440420766104857128" name="SemAnno.structure.Module" flags="ng" index="3rCcZv">
        <property id="8982541288447632592" name="targetLanguage" index="2EAK5_" />
        <child id="8982541288447632589" name="functions" index="2EAK5S" />
      </concept>
      <concept id="8440420766104853160" name="SemAnno.structure.Assignment" flags="ng" index="3rCfXv">
        <child id="8982541288447632365" name="target" index="2EAKpo" />
        <child id="8982541288447632367" name="value" index="2EAKpq" />
      </concept>
    </language>
    <language id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core">
      <concept id="1169194658468" name="jetbrains.mps.lang.core.structure.INamedConcept" flags="ngI" index="TrEIO">
        <property id="1169194664001" name="name" index="TrG5h" />
      </concept>
    </language>
  </registry>
  <node concept="3rCcZv" id="SFE_M001">
    <property role="TrG5h" value="SimpleFunctionExample" />
    <property role="2EAK5_" value="6LZhwXW98Rc/cpp" />
    <node concept="3rCcYv" id="SFE_F001" role="2EAK5S">
      <property role="TrG5h" value="calculate_sum" />
      <node concept="2EAK3$" id="SFE_DR001" role="2EAK55">
        <property role="2EAK3B" value="batched" />
      </node>
      <node concept="3rCcXv" id="SFE_P001" role="2EAK5q">
        <property role="TrG5h" value="a" />
        <node concept="3rC1Zv" id="SFE_PT001" role="2EAK2S">
          <property role="2EAKpV" value="int" />
        </node>
      </node>
      <node concept="3rCcXv" id="SFE_P002" role="2EAK5q">
        <property role="TrG5h" value="b" />
        <node concept="3rC1Zv" id="SFE_PT002" role="2EAK2S">
          <property role="2EAKpV" value="int" />
        </node>
      </node>
      <node concept="3rC1Zv" id="SFE_RT001" role="2EAK5o">
        <property role="2EAKpV" value="int" />
      </node>
      <node concept="3rCfXv" id="SFE_A001" role="2EAK5r">
        <node concept="2EAK2T" id="SFE_VR001" role="2EAKpo">
          <property role="2EAK2V" value="result" />
        </node>
        <node concept="2EAK4o" id="SFE_BO001" role="2EAKpq">
          <property role="TrG5h" value="+" />
          <property role="2EAK4r" value="+" />
          <node concept="2EAK2T" id="SFE_VR_a" role="2EAK4q">
            <property role="2EAK2V" value="a" />
          </node>
          <node concept="2EAK2T" id="SFE_VR_b" role="2EAK45">
            <property role="2EAK2V" value="b" />
          </node>
        </node>
      </node>
      <node concept="2EAK7p" id="SFE_R001" role="2EAK5r">
        <node concept="2EAK2T" id="SFE_VR_ret" role="2EAK7o">
          <property role="2EAK2V" value="result" />
        </node>
      </node>
    </node>
  </node>
</model>

