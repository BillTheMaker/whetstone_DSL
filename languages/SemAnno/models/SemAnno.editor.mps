<?xml version="1.0" encoding="UTF-8"?>
<model ref="r:9e5d413e-df11-44f9-be2a-06f9d8b94fbb(SemAnno.editor)">
  <persistence version="9" />
  <languages>
    <use id="18bc6592-03a6-4e29-a83a-7ff23bde13ba" name="jetbrains.mps.lang.editor" version="15" />
    <use id="aee9cad2-acd4-4608-aef2-0004f6a1cdbd" name="jetbrains.mps.lang.actions" version="4" />
    <devkit ref="fbc25dd2-5da4-483a-8b19-70928e1b62d7(jetbrains.mps.devkit.general-purpose)" />
  </languages>
  <imports>
    <import index="k8se" ref="r:e209a700-5f8c-468c-afcc-f38277628970(SemAnno.structure)" implicit="true" />
    <import index="tpck" ref="r:00000000-0000-4000-0000-011c89590288(jetbrains.mps.lang.core.structure)" implicit="true" />
  </imports>
  <registry>
    <language id="18bc6592-03a6-4e29-a83a-7ff23bde13ba" name="jetbrains.mps.lang.editor">
      <concept id="1071666914219" name="jetbrains.mps.lang.editor.structure.ConceptEditorDeclaration" flags="ig" index="24kQdi" />
      <concept id="1140524381322" name="jetbrains.mps.lang.editor.structure.CellModel_ListWithRole" flags="ng" index="2czfm3">
        <child id="1140524464360" name="cellLayout" index="2czzBx" />
      </concept>
      <concept id="1106270549637" name="jetbrains.mps.lang.editor.structure.CellLayout_Horizontal" flags="nn" index="2iRfu4" />
      <concept id="1106270571710" name="jetbrains.mps.lang.editor.structure.CellLayout_Vertical" flags="nn" index="2iRkQZ" />
      <concept id="1080736578640" name="jetbrains.mps.lang.editor.structure.BaseEditorComponent" flags="ig" index="2wURMF">
        <child id="1080736633877" name="cellModel" index="2wV5jI" />
      </concept>
      <concept id="1088013125922" name="jetbrains.mps.lang.editor.structure.CellModel_RefCell" flags="sg" stub="730538219795941030" index="1iCGBv">
        <child id="1088186146602" name="editorComponent" index="1sWHZn" />
      </concept>
      <concept id="1088185857835" name="jetbrains.mps.lang.editor.structure.InlineEditorComponent" flags="ig" index="1sVBvm" />
      <concept id="1139848536355" name="jetbrains.mps.lang.editor.structure.CellModel_WithRole" flags="ng" index="1$h60E">
        <property id="1140017977771" name="readOnly" index="1Intyy" />
        <reference id="1140103550593" name="relationDeclaration" index="1NtTu8" />
      </concept>
      <concept id="1073389446423" name="jetbrains.mps.lang.editor.structure.CellModel_Collection" flags="sn" stub="3013115976261988961" index="3EZMnI">
        <child id="1106270802874" name="cellLayout" index="2iSdaV" />
        <child id="1073389446424" name="childCellModel" index="3EZMnx" />
      </concept>
      <concept id="1073389577006" name="jetbrains.mps.lang.editor.structure.CellModel_Constant" flags="sn" stub="3610246225209162225" index="3F0ifn">
        <property id="1073389577007" name="text" index="3F0ifm" />
      </concept>
      <concept id="1073389658414" name="jetbrains.mps.lang.editor.structure.CellModel_Property" flags="sg" stub="730538219796134133" index="3F0A7n" />
      <concept id="1073390211982" name="jetbrains.mps.lang.editor.structure.CellModel_RefNodeList" flags="sg" stub="2794558372793454595" index="3F2HdR" />
      <concept id="1166049232041" name="jetbrains.mps.lang.editor.structure.AbstractComponent" flags="ng" index="1XWOmA">
        <reference id="1166049300910" name="conceptDeclaration" index="1XX52x" />
      </concept>
    </language>
  </registry>
  <node concept="24kQdi" id="12Ch1YcKjFC">
    <ref role="1XX52x" to="k8se:12Ch1YcKjFx" resolve="ExecutionMetric" />
    <node concept="3EZMnI" id="12Ch1YcKjFK" role="2wV5jI">
      <node concept="2iRfu4" id="12Ch1YcKjFL" role="2iSdaV" />
      <node concept="3F0ifn" id="12Ch1YcKjFN" role="3EZMnx">
        <property role="3F0ifm" value="Metric: runs=" />
      </node>
      <node concept="3F0A7n" id="4oexHFOhTUi" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:12Ch1YcKjF$" resolve="runcount" />
      </node>
    </node>
  </node>
  <node concept="24kQdi" id="CisPcOYEkB">
    <ref role="1XX52x" to="k8se:CisPcOYEky" resolve="SemanticEntity" />
    <node concept="3EZMnI" id="CisPcOYEkK" role="2wV5jI">
      <node concept="2iRfu4" id="CisPcOYEkL" role="2iSdaV" />
      <node concept="3F0ifn" id="CisPcOYEkI" role="3EZMnx">
        <property role="3F0ifm" value="Entity:" />
      </node>
      <node concept="3F0A7n" id="CisPcOYEkN" role="3EZMnx">
        <ref role="1NtTu8" to="tpck:h0TrG11" resolve="name" />
      </node>
      <node concept="3F2HdR" id="CisPcOYEkS" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:CisPcOYEkA" resolve="components" />
        <node concept="2iRfu4" id="CisPcOYEkU" role="2czzBx" />
      </node>
    </node>
  </node>
  <node concept="24kQdi" id="CisPcOZ8ml">
    <ref role="1XX52x" to="k8se:CisPcOZ8mj" resolve="StringList" />
    <node concept="3EZMnI" id="CisPcOZ8mr" role="2wV5jI">
      <node concept="2iRfu4" id="CisPcOZ8ms" role="2iSdaV" />
      <node concept="3F0ifn" id="CisPcOZ8mp" role="3EZMnx">
        <property role="3F0ifm" value="List [" />
      </node>
      <node concept="3F0A7n" id="CisPcOZ8mw" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:CisPcOZ8mk" resolve="values" />
      </node>
    </node>
  </node>
  <node concept="24kQdi" id="CisPcOZ8mC">
    <ref role="1XX52x" to="k8se:CisPcOZ8mz" resolve="Iterator" />
    <node concept="3EZMnI" id="CisPcOZ8mL" role="2wV5jI">
      <node concept="2iRfu4" id="CisPcOZ8mM" role="2iSdaV" />
      <node concept="3F0ifn" id="CisPcOZ8mI" role="3EZMnx">
        <property role="3F0ifm" value="Iterate over: " />
      </node>
      <node concept="1iCGBv" id="CisPcOZ8mR" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:CisPcOZ8m$" resolve="target" />
        <node concept="1sVBvm" id="CisPcOZ8mT" role="1sWHZn">
          <node concept="3F0A7n" id="CisPcOZ8n0" role="2wV5jI">
            <property role="1Intyy" value="true" />
            <ref role="1NtTu8" to="tpck:h0TrG11" resolve="name" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="24kQdi" id="CisPcP0y_F">
    <ref role="1XX52x" to="k8se:CisPcP0y__" resolve="Script" />
    <node concept="3EZMnI" id="CisPcP0y_S" role="2wV5jI">
      <node concept="2iRkQZ" id="CisPcP0y_T" role="2iSdaV" />
      <node concept="3F0ifn" id="CisPcP0yA1" role="3EZMnx">
        <property role="3F0ifm" value="Script: " />
      </node>
      <node concept="3F0A7n" id="CisPcP0yA6" role="3EZMnx">
        <ref role="1NtTu8" to="tpck:h0TrG11" resolve="name" />
      </node>
      <node concept="3F2HdR" id="CisPcP0AJ9" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:CisPcP0y_C" resolve="entities" />
        <node concept="2iRkQZ" id="CisPcP0AJf" role="2czzBx" />
      </node>
    </node>
  </node>
  <node concept="24kQdi" id="GMyc5g4aVc">
    <ref role="1XX52x" to="k8se:GMyc5g4aRq" resolve="TextLine" />
    <node concept="3F0ifn" id="GMyc5g4aVi" role="2wV5jI">
      <property role="3F0ifm" value="{content}" />
    </node>
  </node>
  <!-- Phase 1: Core AST Structure Editors -->
  <!-- ====================================== -->
  <!-- Module Editor -->
  <node concept="24kQdi" id="PhaseI_ModuleEd">
    <ref role="1XX52x" to="k8se:7kypvuIwECC" resolve="Module" />
    <node concept="3EZMnI" id="PhaseI_ModuleEd_1" role="2wV5jI">
      <node concept="2iRkQZ" id="PhaseI_ModuleEd_L1" role="2iSdaV" />
      <node concept="3F0ifn" id="PhaseI_ModuleEd_K1" role="3EZMnx">
        <property role="3F0ifm" value="module " />
      </node>
      <node concept="3F0A7n" id="PhaseI_ModuleEd_N1" role="3EZMnx">
        <ref role="1NtTu8" to="tpck:h0TrG11" resolve="name" />
      </node>
      <node concept="3F2HdR" id="PhaseI_ModuleEd_F1" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwECF" resolve="functions" />
        <node concept="2iRkQZ" id="PhaseI_ModuleEd_L2" role="2czzBx" />
      </node>
      <node concept="3F2HdR" id="PhaseI_ModuleEd_V1" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwECG" resolve="variables" />
        <node concept="2iRkQZ" id="PhaseI_ModuleEd_L3" role="2czzBx" />
      </node>
    </node>
  </node>
  <!-- Function Editor -->
  <node concept="24kQdi" id="PhaseI_FunctionEd">
    <ref role="1XX52x" to="k8se:7kypvuIwEDC" resolve="Function" />
    <node concept="3EZMnI" id="PhaseI_FunctionEd_1" role="2wV5jI">
      <node concept="2iRkQZ" id="PhaseI_FunctionEd_L1" role="2iSdaV" />
      <node concept="3EZMnI" id="PhaseI_FunctionEd_Sig" role="3EZMnx">
        <node concept="2iRfu4" id="PhaseI_FunctionEd_LS1" role="2iSdaV" />
        <node concept="3F0ifn" id="PhaseI_FunctionEd_def" role="3EZMnx">
          <property role="3F0ifm" value="def " />
        </node>
        <node concept="3F0A7n" id="PhaseI_FunctionEd_FName" role="3EZMnx">
          <ref role="1NtTu8" to="tpck:h0TrG11" resolve="name" />
        </node>
        <node concept="3F0ifn" id="PhaseI_FunctionEd_OP" role="3EZMnx">
          <property role="3F0ifm" value="(" />
        </node>
        <node concept="3F2HdR" id="PhaseI_FunctionEd_Params" role="3EZMnx">
          <ref role="1NtTu8" to="k8se:7kypvuIwEDE" resolve="parameters" />
          <node concept="2iRfu4" id="PhaseI_FunctionEd_LS2" role="2czzBx" />
        </node>
        <node concept="3F0ifn" id="PhaseI_FunctionEd_CP" role="3EZMnx">
          <property role="3F0ifm" value=")" />
        </node>
        <node concept="3F0ifn" id="PhaseI_FunctionEd_Arrow" role="3EZMnx">
          <property role="3F0ifm" value=" -> " />
        </node>
        <node concept="1iCGBv" id="PhaseI_FunctionEd_RetType" role="3EZMnx">
          <ref role="1NtTu8" to="k8se:7kypvuIwEDF" resolve="returnType" />
          <node concept="1sVBvm" id="PhaseI_FunctionEd_RetTypeInline" role="1sWHZn">
            <node concept="3F0A7n" id="PhaseI_FunctionEd_RetTypeVal" role="2wV5jI">
              <property role="1Intyy" value="true" />
              <ref role="1NtTu8" to="tpck:h0TrG11" resolve="name" />
            </node>
          </node>
        </node>
        <node concept="3F0ifn" id="PhaseI_FunctionEd_Colon" role="3EZMnx">
          <property role="3F0ifm" value=":" />
        </node>
      </node>
      <node concept="3F2HdR" id="PhaseI_FunctionEd_Body" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwEDG" resolve="body" />
        <node concept="2iRkQZ" id="PhaseI_FunctionEd_L2" role="2czzBx" />
      </node>
    </node>
  </node>
  <!-- Parameter Editor -->
  <node concept="24kQdi" id="PhaseI_ParameterEd">
    <ref role="1XX52x" to="k8se:7kypvuIwEEC" resolve="Parameter" />
    <node concept="3EZMnI" id="PhaseI_ParameterEd_1" role="2wV5jI">
      <node concept="2iRfu4" id="PhaseI_ParameterEd_L1" role="2iSdaV" />
      <node concept="3F0A7n" id="PhaseI_ParameterEd_Name" role="3EZMnx">
        <ref role="1NtTu8" to="tpck:h0TrG11" resolve="name" />
      </node>
      <node concept="3F0ifn" id="PhaseI_ParameterEd_Colon" role="3EZMnx">
        <property role="3F0ifm" value=": " />
      </node>
      <node concept="1iCGBv" id="PhaseI_ParameterEd_Type" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwEED" resolve="type" />
        <node concept="1sVBvm" id="PhaseI_ParameterEd_TypeInline" role="1sWHZn">
          <node concept="3F0A7n" id="PhaseI_ParameterEd_TypeVal" role="2wV5jI">
            <property role="1Intyy" value="true" />
            <ref role="1NtTu8" to="tpck:h0TrG11" resolve="name" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <!-- Variable Editor -->
  <node concept="24kQdi" id="PhaseI_VariableEd">
    <ref role="1XX52x" to="k8se:7kypvuIwEFC" resolve="Variable" />
    <node concept="3EZMnI" id="PhaseI_VariableEd_1" role="2wV5jI">
      <node concept="2iRfu4" id="PhaseI_VariableEd_L1" role="2iSdaV" />
      <node concept="3F0A7n" id="PhaseI_VariableEd_Name" role="3EZMnx">
        <ref role="1NtTu8" to="tpck:h0TrG11" resolve="name" />
      </node>
      <node concept="3F0ifn" id="PhaseI_VariableEd_Colon" role="3EZMnx">
        <property role="3F0ifm" value=": " />
      </node>
      <node concept="1iCGBv" id="PhaseI_VariableEd_Type" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwEFD" resolve="type" />
        <node concept="1sVBvm" id="PhaseI_VariableEd_TypeInline" role="1sWHZn">
          <node concept="3F0A7n" id="PhaseI_VariableEd_TypeVal" role="2wV5jI">
            <property role="1Intyy" value="true" />
            <ref role="1NtTu8" to="tpck:h0TrG11" resolve="name" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <!-- Block Statement Editor -->
  <node concept="24kQdi" id="PhaseI_BlockEd">
    <ref role="1XX52x" to="k8se:7kypvuIwDDD" resolve="Block" />
    <node concept="3F2HdR" id="PhaseI_BlockEd_1" role="2wV5jI">
      <ref role="1NtTu8" to="k8se:7kypvuIwDDE" resolve="statements" />
      <node concept="2iRkQZ" id="PhaseI_BlockEd_L1" role="2czzBx" />
    </node>
  </node>
  <!-- Assignment Statement Editor -->
  <node concept="24kQdi" id="PhaseI_AssignmentEd">
    <ref role="1XX52x" to="k8se:7kypvuIwDEC" resolve="Assignment" />
    <node concept="3EZMnI" id="PhaseI_AssignmentEd_1" role="2wV5jI">
      <node concept="2iRfu4" id="PhaseI_AssignmentEd_L1" role="2iSdaV" />
      <node concept="1iCGBv" id="PhaseI_AssignmentEd_Target" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwDED" resolve="target" />
        <node concept="1sVBvm" id="PhaseI_AssignmentEd_TargetInline" role="1sWHZn">
          <node concept="3F0A7n" id="PhaseI_AssignmentEd_TargetVal" role="2wV5jI">
            <property role="1Intyy" value="true" />
            <ref role="1NtTu8" to="tpck:h0TrG11" resolve="name" />
          </node>
        </node>
      </node>
      <node concept="3F0ifn" id="PhaseI_AssignmentEd_Eq" role="3EZMnx">
        <property role="3F0ifm" value=" = " />
      </node>
      <node concept="1iCGBv" id="PhaseI_AssignmentEd_Value" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwDEE" resolve="value" />
        <node concept="1sVBvm" id="PhaseI_AssignmentEd_ValueInline" role="1sWHZn" />
      </node>
    </node>
  </node>
  <!-- If Statement Editor -->
  <node concept="24kQdi" id="PhaseI_IfStatementEd">
    <ref role="1XX52x" to="k8se:7kypvuIwDFC" resolve="IfStatement" />
    <node concept="3EZMnI" id="PhaseI_IfStatementEd_1" role="2wV5jI">
      <node concept="2iRkQZ" id="PhaseI_IfStatementEd_L1" role="2iSdaV" />
      <node concept="3EZMnI" id="PhaseI_IfStatementEd_Cond" role="3EZMnx">
        <node concept="2iRfu4" id="PhaseI_IfStatementEd_LS1" role="2iSdaV" />
        <node concept="3F0ifn" id="PhaseI_IfStatementEd_If" role="3EZMnx">
          <property role="3F0ifm" value="if " />
        </node>
        <node concept="1iCGBv" id="PhaseI_IfStatementEd_Condition" role="3EZMnx">
          <ref role="1NtTu8" to="k8se:7kypvuIwDFD" resolve="condition" />
          <node concept="1sVBvm" id="PhaseI_IfStatementEd_CondInline" role="1sWHZn" />
        </node>
        <node concept="3F0ifn" id="PhaseI_IfStatementEd_Colon" role="3EZMnx">
          <property role="3F0ifm" value=":" />
        </node>
      </node>
      <node concept="3F2HdR" id="PhaseI_IfStatementEd_Then" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwDFE" resolve="thenBranch" />
        <node concept="2iRkQZ" id="PhaseI_IfStatementEd_L2" role="2czzBx" />
      </node>
      <node concept="3F2HdR" id="PhaseI_IfStatementEd_Else" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwDFF" resolve="elseBranch" />
        <node concept="2iRkQZ" id="PhaseI_IfStatementEd_L3" role="2czzBx" />
      </node>
    </node>
  </node>
  <!-- While Loop Editor -->
  <node concept="24kQdi" id="PhaseI_WhileLoopEd">
    <ref role="1XX52x" to="k8se:7kypvuIwDGC" resolve="WhileLoop" />
    <node concept="3EZMnI" id="PhaseI_WhileLoopEd_1" role="2wV5jI">
      <node concept="2iRkQZ" id="PhaseI_WhileLoopEd_L1" role="2iSdaV" />
      <node concept="3EZMnI" id="PhaseI_WhileLoopEd_Cond" role="3EZMnx">
        <node concept="2iRfu4" id="PhaseI_WhileLoopEd_LS1" role="2iSdaV" />
        <node concept="3F0ifn" id="PhaseI_WhileLoopEd_While" role="3EZMnx">
          <property role="3F0ifm" value="while " />
        </node>
        <node concept="1iCGBv" id="PhaseI_WhileLoopEd_Condition" role="3EZMnx">
          <ref role="1NtTu8" to="k8se:7kypvuIwDGD" resolve="condition" />
          <node concept="1sVBvm" id="PhaseI_WhileLoopEd_CondInline" role="1sWHZn" />
        </node>
        <node concept="3F0ifn" id="PhaseI_WhileLoopEd_Colon" role="3EZMnx">
          <property role="3F0ifm" value=":" />
        </node>
      </node>
      <node concept="3F2HdR" id="PhaseI_WhileLoopEd_Body" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwDGE" resolve="body" />
        <node concept="2iRkQZ" id="PhaseI_WhileLoopEd_L2" role="2czzBx" />
      </node>
    </node>
  </node>
  <!-- For Loop Editor -->
  <node concept="24kQdi" id="PhaseI_ForLoopEd">
    <ref role="1XX52x" to="k8se:7kypvuIwDHC" resolve="ForLoop" />
    <node concept="3EZMnI" id="PhaseI_ForLoopEd_1" role="2wV5jI">
      <node concept="2iRkQZ" id="PhaseI_ForLoopEd_L1" role="2iSdaV" />
      <node concept="3EZMnI" id="PhaseI_ForLoopEd_Header" role="3EZMnx">
        <node concept="2iRfu4" id="PhaseI_ForLoopEd_LS1" role="2iSdaV" />
        <node concept="3F0ifn" id="PhaseI_ForLoopEd_For" role="3EZMnx">
          <property role="3F0ifm" value="for " />
        </node>
        <node concept="3F0A7n" id="PhaseI_ForLoopEd_Iterator" role="3EZMnx">
          <ref role="1NtTu8" to="k8se:7kypvuIwDHD" resolve="iteratorName" />
        </node>
        <node concept="3F0ifn" id="PhaseI_ForLoopEd_In" role="3EZMnx">
          <property role="3F0ifm" value=" in " />
        </node>
        <node concept="1iCGBv" id="PhaseI_ForLoopEd_Iterable" role="3EZMnx">
          <ref role="1NtTu8" to="k8se:7kypvuIwDHE" resolve="iterable" />
          <node concept="1sVBvm" id="PhaseI_ForLoopEd_IterableInline" role="1sWHZn" />
        </node>
        <node concept="3F0ifn" id="PhaseI_ForLoopEd_Colon" role="3EZMnx">
          <property role="3F0ifm" value=":" />
        </node>
      </node>
      <node concept="3F2HdR" id="PhaseI_ForLoopEd_Body" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwDHF" resolve="body" />
        <node concept="2iRkQZ" id="PhaseI_ForLoopEd_L2" role="2czzBx" />
      </node>
    </node>
  </node>
  <!-- Return Statement Editor -->
  <node concept="24kQdi" id="PhaseI_ReturnEd">
    <ref role="1XX52x" to="k8se:7kypvuIwDIC" resolve="Return" />
    <node concept="3EZMnI" id="PhaseI_ReturnEd_1" role="2wV5jI">
      <node concept="2iRfu4" id="PhaseI_ReturnEd_L1" role="2iSdaV" />
      <node concept="3F0ifn" id="PhaseI_ReturnEd_Return" role="3EZMnx">
        <property role="3F0ifm" value="return " />
      </node>
      <node concept="1iCGBv" id="PhaseI_ReturnEd_Value" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwDID" resolve="value" />
        <node concept="1sVBvm" id="PhaseI_ReturnEd_ValueInline" role="1sWHZn" />
      </node>
    </node>
  </node>
  <!-- Expression Statement Editor -->
  <node concept="24kQdi" id="PhaseI_ExpressionStatementEd">
    <ref role="1XX52x" to="k8se:7kypvuIwDJC" resolve="ExpressionStatement" />
    <node concept="1iCGBv" id="PhaseI_ExpressionStatementEd_1" role="2wV5jI">
      <ref role="1NtTu8" to="k8se:7kypvuIwDJD" resolve="expression" />
      <node concept="1sVBvm" id="PhaseI_ExpressionStatementEd_Inline" role="1sWHZn" />
    </node>
  </node>
  <!-- Binary Operation Expression Editor -->
  <node concept="24kQdi" id="PhaseI_BinaryOpEd">
    <ref role="1XX52x" to="k8se:7kypvuIwCIC2" resolve="BinaryOperation" />
    <node concept="3EZMnI" id="PhaseI_BinaryOpEd_1" role="2wV5jI">
      <node concept="2iRfu4" id="PhaseI_BinaryOpEd_L1" role="2iSdaV" />
      <node concept="1iCGBv" id="PhaseI_BinaryOpEd_Left" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwCIC3" resolve="left" />
        <node concept="1sVBvm" id="PhaseI_BinaryOpEd_LeftInline" role="1sWHZn" />
      </node>
      <node concept="3F0ifn" id="PhaseI_BinaryOpEd_Space1" role="3EZMnx">
        <property role="3F0ifm" value=" " />
      </node>
      <node concept="3F0A7n" id="PhaseI_BinaryOpEd_Op" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwCIC4" resolve="operator" />
      </node>
      <node concept="3F0ifn" id="PhaseI_BinaryOpEd_Space2" role="3EZMnx">
        <property role="3F0ifm" value=" " />
      </node>
      <node concept="1iCGBv" id="PhaseI_BinaryOpEd_Right" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwCIC5" resolve="right" />
        <node concept="1sVBvm" id="PhaseI_BinaryOpEd_RightInline" role="1sWHZn" />
      </node>
    </node>
  </node>
  <!-- Unary Operation Expression Editor -->
  <node concept="24kQdi" id="PhaseI_UnaryOpEd">
    <ref role="1XX52x" to="k8se:7kypvuIwCJC" resolve="UnaryOperation" />
    <node concept="3EZMnI" id="PhaseI_UnaryOpEd_1" role="2wV5jI">
      <node concept="2iRfu4" id="PhaseI_UnaryOpEd_L1" role="2iSdaV" />
      <node concept="3F0A7n" id="PhaseI_UnaryOpEd_Op" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwCJD" resolve="operator" />
      </node>
      <node concept="1iCGBv" id="PhaseI_UnaryOpEd_Operand" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwCJE" resolve="operand" />
        <node concept="1sVBvm" id="PhaseI_UnaryOpEd_OperandInline" role="1sWHZn" />
      </node>
    </node>
  </node>
  <!-- Function Call Expression Editor -->
  <node concept="24kQdi" id="PhaseI_FunctionCallEd">
    <ref role="1XX52x" to="k8se:7kypvuIwCLC" resolve="FunctionCall" />
    <node concept="3EZMnI" id="PhaseI_FunctionCallEd_1" role="2wV5jI">
      <node concept="2iRfu4" id="PhaseI_FunctionCallEd_L1" role="2iSdaV" />
      <node concept="3F0A7n" id="PhaseI_FunctionCallEd_Name" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwCLD" resolve="functionName" />
      </node>
      <node concept="3F0ifn" id="PhaseI_FunctionCallEd_OP" role="3EZMnx">
        <property role="3F0ifm" value="(" />
      </node>
      <node concept="3F2HdR" id="PhaseI_FunctionCallEd_Args" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwCLE" resolve="arguments" />
        <node concept="2iRfu4" id="PhaseI_FunctionCallEd_L2" role="2czzBx" />
      </node>
      <node concept="3F0ifn" id="PhaseI_FunctionCallEd_CP" role="3EZMnx">
        <property role="3F0ifm" value=")" />
      </node>
    </node>
  </node>
  <!-- Variable Reference Expression Editor -->
  <node concept="24kQdi" id="PhaseI_VariableRefEd">
    <ref role="1XX52x" to="k8se:7kypvuIwCKC" resolve="VariableReference" />
    <node concept="3F0A7n" id="PhaseI_VariableRefEd_1" role="2wV5jI">
      <ref role="1NtTu8" to="k8se:7kypvuIwCKD" resolve="variableName" />
    </node>
  </node>
  <!-- Integer Literal Expression Editor -->
  <node concept="24kQdi" id="PhaseI_IntLitEd">
    <ref role="1XX52x" to="k8se:7kypvuIwCDC" resolve="IntegerLiteral" />
    <node concept="3F0A7n" id="PhaseI_IntLitEd_1" role="2wV5jI">
      <ref role="1NtTu8" to="k8se:7kypvuIwCDD" resolve="value" />
    </node>
  </node>
  <!-- Float Literal Expression Editor -->
  <node concept="24kQdi" id="PhaseI_FloatLitEd">
    <ref role="1XX52x" to="k8se:7kypvuIwCEC" resolve="FloatLiteral" />
    <node concept="3F0A7n" id="PhaseI_FloatLitEd_1" role="2wV5jI">
      <ref role="1NtTu8" to="k8se:7kypvuIwCED" resolve="value" />
    </node>
  </node>
  <!-- String Literal Expression Editor -->
  <node concept="24kQdi" id="PhaseI_StringLitEd">
    <ref role="1XX52x" to="k8se:7kypvuIwCFC" resolve="StringLiteral" />
    <node concept="3EZMnI" id="PhaseI_StringLitEd_1" role="2wV5jI">
      <node concept="2iRfu4" id="PhaseI_StringLitEd_L1" role="2iSdaV" />
      <node concept="3F0ifn" id="PhaseI_StringLitEd_Q1" role="3EZMnx">
        <property role="3F0ifm" value="\"" />
      </node>
      <node concept="3F0A7n" id="PhaseI_StringLitEd_Val" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwCFD" resolve="value" />
      </node>
      <node concept="3F0ifn" id="PhaseI_StringLitEd_Q2" role="3EZMnx">
        <property role="3F0ifm" value="\"" />
      </node>
    </node>
  </node>
  <!-- Boolean Literal Expression Editor -->
  <node concept="24kQdi" id="PhaseI_BoolLitEd">
    <ref role="1XX52x" to="k8se:7kypvuIwCGC" resolve="BooleanLiteral" />
    <node concept="3F0A7n" id="PhaseI_BoolLitEd_1" role="2wV5jI">
      <ref role="1NtTu8" to="k8se:7kypvuIwCGD" resolve="value" />
    </node>
  </node>
  <!-- Null Literal Expression Editor -->
  <node concept="24kQdi" id="PhaseI_NullLitEd">
    <ref role="1XX52x" to="k8se:7kypvuIwCHC" resolve="NullLiteral" />
    <node concept="3F0ifn" id="PhaseI_NullLitEd_1" role="2wV5jI">
      <property role="3F0ifm" value="null" />
    </node>
  </node>
  <!-- List Literal Expression Editor -->
  <node concept="24kQdi" id="PhaseI_ListLitEd">
    <ref role="1XX52x" to="k8se:7kypvuIwCMC" resolve="ListLiteral" />
    <node concept="3EZMnI" id="PhaseI_ListLitEd_1" role="2wV5jI">
      <node concept="2iRfu4" id="PhaseI_ListLitEd_L1" role="2iSdaV" />
      <node concept="3F0ifn" id="PhaseI_ListLitEd_OB" role="3EZMnx">
        <property role="3F0ifm" value="[" />
      </node>
      <node concept="3F2HdR" id="PhaseI_ListLitEd_Elements" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwCMD" resolve="elements" />
        <node concept="2iRfu4" id="PhaseI_ListLitEd_L2" role="2czzBx" />
      </node>
      <node concept="3F0ifn" id="PhaseI_ListLitEd_CB" role="3EZMnx">
        <property role="3F0ifm" value="]" />
      </node>
    </node>
  </node>
  <!-- Index Access Expression Editor -->
  <node concept="24kQdi" id="PhaseI_IndexAccessEd">
    <ref role="1XX52x" to="k8se:7kypvuIwCNC" resolve="IndexAccess" />
    <node concept="3EZMnI" id="PhaseI_IndexAccessEd_1" role="2wV5jI">
      <node concept="2iRfu4" id="PhaseI_IndexAccessEd_L1" role="2iSdaV" />
      <node concept="1iCGBv" id="PhaseI_IndexAccessEd_Target" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwCND" resolve="target" />
        <node concept="1sVBvm" id="PhaseI_IndexAccessEd_TargetInline" role="1sWHZn" />
      </node>
      <node concept="3F0ifn" id="PhaseI_IndexAccessEd_OB" role="3EZMnx">
        <property role="3F0ifm" value="[" />
      </node>
      <node concept="1iCGBv" id="PhaseI_IndexAccessEd_Index" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwCNE" resolve="index" />
        <node concept="1sVBvm" id="PhaseI_IndexAccessEd_IndexInline" role="1sWHZn" />
      </node>
      <node concept="3F0ifn" id="PhaseI_IndexAccessEd_CB" role="3EZMnx">
        <property role="3F0ifm" value="]" />
      </node>
    </node>
  </node>
  <!-- Member Access Expression Editor -->
  <node concept="24kQdi" id="PhaseI_MemberAccessEd">
    <ref role="1XX52x" to="k8se:7kypvuIwCOC" resolve="MemberAccess" />
    <node concept="3EZMnI" id="PhaseI_MemberAccessEd_1" role="2wV5jI">
      <node concept="2iRfu4" id="PhaseI_MemberAccessEd_L1" role="2iSdaV" />
      <node concept="1iCGBv" id="PhaseI_MemberAccessEd_Target" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwCOD" resolve="target" />
        <node concept="1sVBvm" id="PhaseI_MemberAccessEd_TargetInline" role="1sWHZn" />
      </node>
      <node concept="3F0ifn" id="PhaseI_MemberAccessEd_Dot" role="3EZMnx">
        <property role="3F0ifm" value="." />
      </node>
      <node concept="3F0A7n" id="PhaseI_MemberAccessEd_Member" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwCOE" resolve="memberName" />
      </node>
    </node>
  </node>
  <!-- Primitive Type Editor -->
  <node concept="24kQdi" id="PhaseI_PrimitiveTypeEd">
    <ref role="1XX52x" to="k8se:7kypvuIwBCC" resolve="PrimitiveType" />
    <node concept="3F0A7n" id="PhaseI_PrimitiveTypeEd_1" role="2wV5jI">
      <ref role="1NtTu8" to="k8se:7kypvuIwBCD" resolve="kind" />
    </node>
  </node>
  <!-- List Type Editor -->
  <node concept="24kQdi" id="PhaseI_ListTypeEd">
    <ref role="1XX52x" to="k8se:7kypvuIwBDC" resolve="ListType" />
    <node concept="3EZMnI" id="PhaseI_ListTypeEd_1" role="2wV5jI">
      <node concept="2iRfu4" id="PhaseI_ListTypeEd_L1" role="2iSdaV" />
      <node concept="3F0ifn" id="PhaseI_ListTypeEd_List" role="3EZMnx">
        <property role="3F0ifm" value="list[" />
      </node>
      <node concept="1iCGBv" id="PhaseI_ListTypeEd_ElemType" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwBDD" resolve="elementType" />
        <node concept="1sVBvm" id="PhaseI_ListTypeEd_ElemTypeInline" role="1sWHZn" />
      </node>
      <node concept="3F0ifn" id="PhaseI_ListTypeEd_CB" role="3EZMnx">
        <property role="3F0ifm" value="]" />
      </node>
    </node>
  </node>
  <!-- Optional Type Editor -->
  <node concept="24kQdi" id="PhaseI_OptionalTypeEd">
    <ref role="1XX52x" to="k8se:7kypvuIwBFC" resolve="OptionalType" />
    <node concept="3EZMnI" id="PhaseI_OptionalTypeEd_1" role="2wV5jI">
      <node concept="2iRfu4" id="PhaseI_OptionalTypeEd_L1" role="2iSdaV" />
      <node concept="3F0ifn" id="PhaseI_OptionalTypeEd_Optional" role="3EZMnx">
        <property role="3F0ifm" value="optional[" />
      </node>
      <node concept="1iCGBv" id="PhaseI_OptionalTypeEd_InnerType" role="3EZMnx">
        <ref role="1NtTu8" to="k8se:7kypvuIwBFD" resolve="innerType" />
        <node concept="1sVBvm" id="PhaseI_OptionalTypeEd_InnerTypeInline" role="1sWHZn" />
      </node>
      <node concept="3F0ifn" id="PhaseI_OptionalTypeEd_CB" role="3EZMnx">
        <property role="3F0ifm" value="]" />
      </node>
    </node>
  </node>
  <!-- Custom Type Editor -->
  <node concept="24kQdi" id="PhaseI_CustomTypeEd">
    <ref role="1XX52x" to="k8se:7kypvuIwBJC" resolve="CustomType" />
    <node concept="3F0A7n" id="PhaseI_CustomTypeEd_1" role="2wV5jI">
      <ref role="1NtTu8" to="k8se:7kypvuIwBJD" resolve="typeName" />
    </node>
  </node>
</model>

