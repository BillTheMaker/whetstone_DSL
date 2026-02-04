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
      <concept id="8440420766104857128" name="SemAnno.structure.Module" flags="ng" index="Module_1">
        <child id="8982541288447632590" name="variables" index="Module_v" />
        <child id="8982541288447632589" name="functions" index="Module_f" />
      </concept>
      <concept id="8440420766104857192" name="SemAnno.structure.Function" flags="ng" index="Func_1">
        <child id="8982541288447632623" name="parameters" index="Func_p" />
        <child id="8982541288447632622" name="body" index="Func_b" />
        <reference id="8982541288447632621" name="returnType" index="Func_rt" />
      </concept>
      <concept id="8440420766104857256" name="SemAnno.structure.Parameter" flags="ng" index="Param_1">
        <reference id="8982541288447632653" name="type" index="Param_t" />
      </concept>
      <concept id="8982541288447632700" name="SemAnno.structure.Variable" flags="ng" index="Var_1" />
      <concept id="8982541288447632333" name="SemAnno.structure.Block" flags="ng" index="Block_1">
        <child id="8982541288447632334" name="statements" index="Block_s" />
      </concept>
      <concept id="8440420766104853160" name="SemAnno.structure.Assignment" flags="ng" index="Assign_1">
        <reference id="8982541288447632365" name="target" index="Assign_t" />
        <reference id="8982541288447632366" name="value" index="Assign_v" />
      </concept>
      <concept id="8982541288447632557" name="SemAnno.structure.BinaryOperation" flags="ng" index="BinOp_1">
        <reference id="8982541288447632559" name="left" index="BinOp_l" />
        <reference id="8982541288447632560" name="right" index="BinOp_r" />
      </concept>
      <concept id="8982541288447632652" name="SemAnno.structure.VariableReference" flags="ng" index="VarRef_1">
        <property id="8982541288447632653" name="variableName" index="VarRef_n" />
      </concept>
      <concept id="8982541288447632492" name="SemAnno.structure.Return" flags="ng" index="Return_1">
        <reference id="8982541288447632493" name="value" index="Return_v" />
      </concept>
      <concept id="8440420766104844840" name="SemAnno.structure.PrimitiveType" flags="ng" index="PrimT_1">
        <property id="8982541288447632334" name="kind" index="PrimT_k" />
      </concept>
      <concept id="8440420766104853480" name="SemAnno.structure.ExpressionStatement" flags="ng" index="ExprStmt_1">
        <reference id="8982541288447632525" name="expression" index="ExprStmt_e" />
      </concept>
    </language>
    <language id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core">
      <concept id="1169194658468" name="jetbrains.mps.lang.core.structure.INamedConcept" flags="ngI" index="TrEIO">
        <property id="1169194664001" name="name" index="TrG5h" />
      </concept>
    </language>
  </registry>

  <!-- Shared Type nodes (referenced by parameters and functions) -->
  <node concept="PrimT_1" id="Type_Int">
    <property role="PrimT_k" value="int" />
  </node>

  <!-- Calculator Module -->
  <node concept="Module_1" id="Calc_M001">
    <property role="TrG5h" value="Calculator" />

    <!-- Variable: PI -->
    <node concept="Var_1" id="Calc_V001" role="Module_v">
      <property role="TrG5h" value="PI" />
    </node>

    <!-- Function: add(x, y) -->
    <node concept="Func_1" id="Calc_F001" role="Module_f">
      <property role="TrG5h" value="add" />
      <ref role="Func_rt" node="Type_Int" />

      <!-- Parameter x -->
      <node concept="Param_1" id="Calc_P001" role="Func_p">
        <property role="TrG5h" value="x" />
        <ref role="Param_t" node="Type_Int" />
      </node>

      <!-- Parameter y -->
      <node concept="Param_1" id="Calc_P002" role="Func_p">
        <property role="TrG5h" value="y" />
        <ref role="Param_t" node="Type_Int" />
      </node>

      <!-- Body -->
      <node concept="Block_1" id="Calc_B001" role="Func_b">
        <!-- result = x + y -->
        <node concept="Assign_1" id="Calc_A001" role="Block_s">
          <ref role="Assign_t" node="Calc_VR_result" />
          <ref role="Assign_v" node="Calc_E001" />
        </node>
        <!-- return result -->
        <node concept="Return_1" id="Calc_R001" role="Block_s">
          <ref role="Return_v" node="Calc_VR_return" />
        </node>
      </node>

      <!-- Expression nodes for add function -->
      <node concept="VarRef_1" id="Calc_VR_result">
        <property role="VarRef_n" value="result" />
      </node>
      <node concept="BinOp_1" id="Calc_E001">
        <ref role="BinOp_l" node="Calc_VR_x" />
        <ref role="BinOp_r" node="Calc_VR_y" />
      </node>
      <node concept="VarRef_1" id="Calc_VR_x">
        <property role="VarRef_n" value="x" />
      </node>
      <node concept="VarRef_1" id="Calc_VR_y">
        <property role="VarRef_n" value="y" />
      </node>
      <node concept="VarRef_1" id="Calc_VR_return">
        <property role="VarRef_n" value="result" />
      </node>
    </node>

    <!-- Function: multiply(a, b) -->
    <node concept="Func_1" id="Calc_F002" role="Module_f">
      <property role="TrG5h" value="multiply" />
      <ref role="Func_rt" node="Type_Int" />

      <!-- Parameter a -->
      <node concept="Param_1" id="Calc_P003" role="Func_p">
        <property role="TrG5h" value="a" />
        <ref role="Param_t" node="Type_Int" />
      </node>

      <!-- Parameter b -->
      <node concept="Param_1" id="Calc_P004" role="Func_p">
        <property role="TrG5h" value="b" />
        <ref role="Param_t" node="Type_Int" />
      </node>

      <!-- Body with ExpressionStatement wrapping BinaryOperation -->
      <node concept="Block_1" id="Calc_B002" role="Func_b">
        <node concept="ExprStmt_1" id="Calc_ES001" role="Block_s">
          <ref role="ExprStmt_e" node="Calc_E005" />
        </node>
      </node>

      <!-- Expression nodes for multiply function -->
      <node concept="BinOp_1" id="Calc_E005">
        <ref role="BinOp_l" node="Calc_VR_a" />
        <ref role="BinOp_r" node="Calc_VR_b" />
      </node>
      <node concept="VarRef_1" id="Calc_VR_a">
        <property role="VarRef_n" value="a" />
      </node>
      <node concept="VarRef_1" id="Calc_VR_b">
        <property role="VarRef_n" value="b" />
      </node>
    </node>
  </node>
</model>
