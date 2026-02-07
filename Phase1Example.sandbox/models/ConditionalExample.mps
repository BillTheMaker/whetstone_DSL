<?xml version="1.0" encoding="UTF-8"?>
<model ref="r:b2c3d4e5-f6a7-48b9-c0d1-e2f3a4b5c6d7(ConditionalExample)">
  <persistence version="9" />
  <languages>
    <use id="a452e1cf-6f8c-4bca-8447-fd4d6e4d0003" name="SemAnno" version="0" />
    <use id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core" version="2" />
  </languages>
  <imports />
  <registry>
    <language id="a452e1cf-6f8c-4bca-8447-fd4d6e4d0003" name="SemAnno">
      <concept id="8982541288447632557" name="SemAnno.structure.BinaryOperation" flags="ng" index="2EAK4o">
        <property id="8982541288447632558" name="operator" index="2EAK4r" />
        <child id="8982541288447632560" name="right" index="2EAK45" />
        <child id="8982541288447632559" name="left" index="2EAK4q" />
      </concept>
      <concept id="8982541288447632556" name="SemAnno.structure.IntegerLiteral" flags="ng" index="2EAK4p">
        <property id="8982541288447632559" name="value" index="2EAK4o" />
      </concept>
      <concept id="8982541288447632572" name="SemAnno.structure.StringLiteral" flags="ng" index="2EAK49">
        <property id="8982541288447632573" name="value" index="2EAK48" />
      </concept>
      <concept id="8982541288447632652" name="SemAnno.structure.VariableReference" flags="ng" index="2EAK2T">
        <property id="8982541288447632654" name="variableName" index="2EAK2S" />
      </concept>
      <concept id="8982541288447632492" name="SemAnno.structure.Return" flags="ng" index="2EAK7p">
        <child id="8982541288447632493" name="value" index="2EAK7o" />
      </concept>
      <concept id="8982541288447632721" name="SemAnno.structure.DerefStrategy" flags="ng" index="2EAK5o">
        <property id="8982541288447632722" name="strategy" index="2EAK5r" />
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
        <child id="8982541288447632624" name="annotations" index="2EAK55" />
      </concept>
      <concept id="8440420766104857128" name="SemAnno.structure.Module" flags="ng" index="3rCcZv">
        <property id="8982541288447632592" name="targetLanguage" index="2EAK5_" />
        <child id="8982541288447632589" name="functions" index="2EAK5S" />
        <child id="8982541288447632592" name="variables" index="2EAK5V" />
      </concept>
      <concept id="8440420766104853224" name="SemAnno.structure.IfStatement" flags="ng" index="3rCfWv">
        <child id="8982541288447632397" name="condition" index="2EAK6S" />
        <child id="8982541288447632399" name="thenBranch" index="2EAK6V" />
        <child id="8982541288447632400" name="elseBranch" index="2EAK6U" />
      </concept>
    </language>
    <language id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core">
      <concept id="1169194658468" name="jetbrains.mps.lang.core.structure.INamedConcept" flags="ngI" index="TrEIO">
        <property id="1169194664001" name="name" index="TrG5h" />
      </concept>
    </language>
  </registry>
  <node concept="3rCcZv" id="CE_M001">
    <property role="TrG5h" value="ConditionalExample" />
    <property role="2EAK5_" value="6LZhwXW98Xr/both" />
    <node concept="3rCcYv" id="CE_F001" role="2EAK5S">
      <property role="TrG5h" value="check_status" />
      <node concept="2EAK5o" id="CE_DR001" role="2EAK55">
        <property role="2EAK5r" value="batched" />
      </node>
      <node concept="3rCcXv" id="CE_P001" role="2EAK5q">
        <property role="TrG5h" value="status_code" />
        <node concept="3rC1Zv" id="CE_PT001" role="2EAK2S">
          <property role="2EAKpV" value="int" />
        </node>
      </node>
      <node concept="3rC1Zv" id="CE_RT001" role="2EAK5o">
        <property role="2EAKpV" value="string" />
      </node>
      <node concept="3rCfWv" id="CE_IF001" role="2EAK5r">
        <node concept="2EAK4o" id="CE_BO001" role="2EAK6S">
          <property role="TrG5h" value="==" />
          <property role="2EAK4r" value="==" />
          <node concept="2EAK2T" id="CE_VR001" role="2EAK4q">
            <property role="2EAK2S" value="status_code" />
          </node>
          <node concept="2EAK4p" id="CE_IL001" role="2EAK45">
            <property role="2EAK4o" value="200" />
          </node>
        </node>
        <node concept="2EAK7p" id="CE_R001" role="2EAK6V">
          <node concept="2EAK49" id="CE_SL001" role="2EAK7o">
            <property role="2EAK48" value="OK" />
          </node>
        </node>
        <node concept="2EAK7p" id="CE_R002" role="2EAK6U">
          <node concept="2EAK49" id="CE_SL002" role="2EAK7o">
            <property role="2EAK48" value="Error" />
          </node>
        </node>
      </node>
    </node>
  </node>
</model>
