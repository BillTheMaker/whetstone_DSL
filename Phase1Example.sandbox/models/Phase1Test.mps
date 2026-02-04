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
      <!-- Core Concepts -->
      <concept id="8440420766104857128" name="SemAnno.structure.Module" flags="ng" index="2Ks9RF">
        <child id="8982541288447632589" name="variables" index="2Ks9RG" />
        <child id="8982541288447632588" name="functions" index="2Ks9RH" />
      </concept>
      <concept id="8982541288447632700" name="SemAnno.structure.Variable" flags="ng" index="2KtM7O" />
      <concept id="8440420766104857192" name="SemAnno.structure.Function" flags="ng" index="2Ks9S5">
        <child id="8982541288447632614" name="parameters" index="2Ks9SI" />
        <child id="8982541288447632625" name="body" index="2Ks9SJ" />
        <child id="8982541288447632624" name="returnType" index="2Ks9SK" />
      </concept>
      <concept id="8440420766104857256" name="SemAnno.structure.Parameter" flags="ng" index="2Ks9Tj">
        <child id="8982541288447632623" name="type" index="2Ks9SL" />
      </concept>

      <!-- Statements -->
      <concept id="8982541288447632647" name="SemAnno.structure.Block" flags="ng" index="2KtLg9">
        <child id="8982541288447632648" name="statements" index="2KtLgA" />
      </concept>
      <concept id="8982541288447632660" name="SemAnno.structure.Assignment" flags="ng" index="2KtLgs">
        <child id="8982541288447632661" name="value" index="2KtLgt" />
      </concept>
      <concept id="8982541288447632686" name="SemAnno.structure.IfStatement" flags="ng" index="2KtLhQ">
        <child id="8982541288447632687" name="condition" index="2KtLhR" />
        <child id="8982541288447632688" name="thenBranch" index="2KtLhS" />
        <child id="8982541288447632689" name="elseBranch" index="2KtLhT" />
      </concept>
      <concept id="8982541288447632705" name="SemAnno.structure.WhileLoop" flags="ng" index="2KtLiZ">
        <child id="8982541288447632706" name="condition" index="2KtLjA" />
        <child id="8982541288447632707" name="body" index="2KtLjB" />
      </concept>
      <concept id="8982541288447632753" name="SemAnno.structure.Return" flags="ng" index="2KtLkJ">
        <child id="8982541288447632754" name="value" index="2KtLkK" />
      </concept>

      <!-- Expressions -->
      <concept id="8982541288447632765" name="SemAnno.structure.Expression" flags="ng" index="2KtLkV" />
      <concept id="8982541288447632764" name="SemAnno.structure.BinaryOperation" flags="ng" index="2KtLkU">
        <child id="8982541288447632766" name="left" index="2KtLkW" />
        <child id="8982541288447632767" name="right" index="2KtLkX" />
      </concept>
      <concept id="8982541288447632817" name="SemAnno.structure.VariableReference" flags="ng" index="2KtLnB" />
      <concept id="8982541288447632837" name="SemAnno.structure.IntegerLiteral" flags="ng" index="2KtLoP" />
      <concept id="8982541288447632897" name="SemAnno.structure.BooleanLiteral" flags="ng" index="2KtLrZ" />

      <!-- Types -->
      <concept id="8982541288447632576" name="SemAnno.structure.Type" flags="ng" index="2KtL_8" />
      <concept id="8982541288447632596" name="SemAnno.structure.PrimitiveType" flags="ng" index="2KtL_I" />
    </language>
    <language id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core">
      <concept id="1169194658468" name="jetbrains.mps.lang.core.structure.INamedConcept" flags="ngI" index="TrEIO">
        <property id="1169194664001" name="name" index="TrG5h" />
      </concept>
    </language>
  </registry>

  <node concept="2Ks9RF" id="Phase1_M001">
    <property role="TrG5h" value="Calculator" />

    <!-- Global variables -->
    <node concept="2KtM7O" id="Phase1_V001" role="2Ks9RG">
      <property role="TrG5h" value="PI" />
    </node>

    <!-- Function: add two numbers -->
    <node concept="2Ks9S5" id="Phase1_F001" role="2Ks9RH">
      <property role="TrG5h" value="add" />
      <node concept="2Ks9Tj" id="Phase1_P001" role="2Ks9SI">
        <property role="TrG5h" value="x" />
        <node concept="2KtL_I" id="Phase1_T001" role="2Ks9SL">
          <property role="TrG5h" value="int" />
        </node>
      </node>
      <node concept="2Ks9Tj" id="Phase1_P002" role="2Ks9SI">
        <property role="TrG5h" value="y" />
        <node concept="2KtL_I" id="Phase1_T002" role="2Ks9SL">
          <property role="TrG5h" value="int" />
        </node>
      </node>
      <node concept="2KtL_I" id="Phase1_T003" role="2Ks9SK">
        <property role="TrG5h" value="int" />
      </node>
      <node concept="2KtLg9" id="Phase1_B001" role="2Ks9SJ">
        <node concept="2KtLgs" id="Phase1_A001" role="2KtLgA">
          <property role="TrG5h" value="result" />
          <node concept="2KtLkU" id="Phase1_E001" role="2KtLgt">
            <property role="TrG5h" value="+" />
            <node concept="2KtLnB" id="Phase1_E002" role="2KtLkW">
              <property role="TrG5h" value="x" />
            </node>
            <node concept="2KtLnB" id="Phase1_E003" role="2KtLkX">
              <property role="TrG5h" value="y" />
            </node>
          </node>
        </node>
        <node concept="2KtLkJ" id="Phase1_R001" role="2KtLgA">
          <node concept="2KtLnB" id="Phase1_E004" role="2KtLkK">
            <property role="TrG5h" value="result" />
          </node>
        </node>
      </node>
    </node>

    <!-- Function: check if positive -->
    <node concept="2Ks9S5" id="Phase1_F002" role="2Ks9RH">
      <property role="TrG5h" value="isPositive" />
      <node concept="2Ks9Tj" id="Phase1_P003" role="2Ks9SI">
        <property role="TrG5h" value="num" />
        <node concept="2KtL_I" id="Phase1_T004" role="2Ks9SL">
          <property role="TrG5h" value="int" />
        </node>
      </node>
      <node concept="2KtL_I" id="Phase1_T005" role="2Ks9SK">
        <property role="TrG5h" value="bool" />
      </node>
      <node concept="2KtLg9" id="Phase1_B002" role="2Ks9SJ">
        <node concept="2KtLhQ" id="Phase1_IF001" role="2KtLgA">
          <node concept="2KtLkU" id="Phase1_E005" role="2KtLhR">
            <property role="TrG5h" value=">" />
            <node concept="2KtLnB" id="Phase1_E006" role="2KtLkW">
              <property role="TrG5h" value="num" />
            </node>
            <node concept="2KtLoP" id="Phase1_E007" role="2KtLkX">
              <property role="TrG5h" value="0" />
            </node>
          </node>
          <node concept="2KtLg9" id="Phase1_B003" role="2KtLhS">
            <node concept="2KtLkJ" id="Phase1_R002" role="2KtLgA">
              <node concept="2KtLrZ" id="Phase1_E008" role="2KtLkK">
                <property role="TrG5h" value="true" />
              </node>
            </node>
          </node>
          <node concept="2KtLg9" id="Phase1_B004" role="2KtLhT">
            <node concept="2KtLkJ" id="Phase1_R003" role="2KtLgA">
              <node concept="2KtLrZ" id="Phase1_E009" role="2KtLkK">
                <property role="TrG5h" value="false" />
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>

    <!-- Function: sum with loop -->
    <node concept="2Ks9S5" id="Phase1_F003" role="2Ks9RH">
      <property role="TrG5h" value="sumTo" />
      <node concept="2Ks9Tj" id="Phase1_P004" role="2Ks9SI">
        <property role="TrG5h" value="n" />
        <node concept="2KtL_I" id="Phase1_T006" role="2Ks9SL">
          <property role="TrG5h" value="int" />
        </node>
      </node>
      <node concept="2KtL_I" id="Phase1_T007" role="2Ks9SK">
        <property role="TrG5h" value="int" />
      </node>
      <node concept="2KtLg9" id="Phase1_B005" role="2Ks9SJ">
        <node concept="2KtLgs" id="Phase1_A002" role="2KtLgA">
          <property role="TrG5h" value="sum" />
          <node concept="2KtLoP" id="Phase1_E010" role="2KtLgt">
            <property role="TrG5h" value="0" />
          </node>
        </node>
        <node concept="2KtLiZ" id="Phase1_W001" role="2KtLgA">
          <node concept="2KtLkU" id="Phase1_E011" role="2KtLjA">
            <property role="TrG5h" value="<" />
            <node concept="2KtLoP" id="Phase1_E012" role="2KtLkW">
              <property role="TrG5h" value="0" />
            </node>
            <node concept="2KtLnB" id="Phase1_E013" role="2KtLkX">
              <property role="TrG5h" value="n" />
            </node>
          </node>
          <node concept="2KtLg9" id="Phase1_B006" role="2KtLjB">
            <node concept="2KtLgs" id="Phase1_A003" role="2KtLgA">
              <property role="TrG5h" value="sum" />
              <node concept="2KtLkU" id="Phase1_E014" role="2KtLgt">
                <property role="TrG5h" value="+" />
                <node concept="2KtLnB" id="Phase1_E015" role="2KtLkW">
                  <property role="TrG5h" value="sum" />
                </node>
                <node concept="2KtLoP" id="Phase1_E016" role="2KtLkX">
                  <property role="TrG5h" value="1" />
                </node>
              </node>
            </node>
          </node>
        </node>
        <node concept="2KtLkJ" id="Phase1_R004" role="2KtLgA">
          <node concept="2KtLnB" id="Phase1_E017" role="2KtLkK">
            <property role="TrG5h" value="sum" />
          </node>
        </node>
      </node>
    </node>
  </node>
</model>
