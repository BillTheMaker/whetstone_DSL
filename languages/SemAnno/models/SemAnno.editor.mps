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

  <!-- Phase 1: Core AST Structure Editors (31 concepts) -->

  <!-- Module Editor: module [name] with functions/variables -->
  <node concept="24kQdi" id="Ed_10001">
    <ref role="1XX52x" to="k8se:7kypvuIwECC" resolve="Module" />
    <node concept="3EZMnI" id="Ed_10002" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10003" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10004" role="3EZMnx">
        <property role="3F0ifm" value="Module: " />
      </node>
    </node>
  </node>

  <!-- Function Editor: def [name]([params]) -> [type]: -->
  <node concept="24kQdi" id="Ed_10005">
    <ref role="1XX52x" to="k8se:7kypvuIwEDC" resolve="Function" />
    <node concept="3EZMnI" id="Ed_10006" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10007" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10008" role="3EZMnx">
        <property role="3F0ifm" value="Function: " />
      </node>
    </node>
  </node>

  <!-- Parameter Editor: [name]: [type] -->
  <node concept="24kQdi" id="Ed_10009">
    <ref role="1XX52x" to="k8se:7kypvuIwEEC" resolve="Parameter" />
    <node concept="3EZMnI" id="Ed_10010" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10011" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10012" role="3EZMnx">
        <property role="3F0ifm" value="Parameter: " />
      </node>
    </node>
  </node>

  <!-- Variable Editor: [name]: [type] -->
  <node concept="24kQdi" id="Ed_10013">
    <ref role="1XX52x" to="k8se:7kypvuIwEFC" resolve="Variable" />
    <node concept="3EZMnI" id="Ed_10014" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10015" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10016" role="3EZMnx">
        <property role="3F0ifm" value="Variable: " />
      </node>
    </node>
  </node>

  <!-- Block Editor: statements list -->
  <node concept="24kQdi" id="Ed_10017">
    <ref role="1XX52x" to="k8se:7kypvuIwDDD" resolve="Block" />
    <node concept="3EZMnI" id="Ed_10018" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10019" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10020" role="3EZMnx">
        <property role="3F0ifm" value="Block: " />
      </node>
    </node>
  </node>

  <!-- Assignment Editor: [target] = [value] -->
  <node concept="24kQdi" id="Ed_10021">
    <ref role="1XX52x" to="k8se:7kypvuIwDEC" resolve="Assignment" />
    <node concept="3EZMnI" id="Ed_10022" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10023" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10024" role="3EZMnx">
        <property role="3F0ifm" value="Assignment: " />
      </node>
    </node>
  </node>

  <!-- IfStatement Editor: if [condition]: [then] else: [else] -->
  <node concept="24kQdi" id="Ed_10025">
    <ref role="1XX52x" to="k8se:7kypvuIwDFC" resolve="IfStatement" />
    <node concept="3EZMnI" id="Ed_10026" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10027" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10028" role="3EZMnx">
        <property role="3F0ifm" value="IfStatement: " />
      </node>
    </node>
  </node>

  <!-- WhileLoop Editor: while [condition]: [body] -->
  <node concept="24kQdi" id="Ed_10029">
    <ref role="1XX52x" to="k8se:7kypvuIwDGC" resolve="WhileLoop" />
    <node concept="3EZMnI" id="Ed_10030" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10031" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10032" role="3EZMnx">
        <property role="3F0ifm" value="WhileLoop: " />
      </node>
    </node>
  </node>

  <!-- ForLoop Editor: for [iterator] in [iterable]: [body] -->
  <node concept="24kQdi" id="Ed_10033">
    <ref role="1XX52x" to="k8se:7kypvuIwDHC" resolve="ForLoop" />
    <node concept="3EZMnI" id="Ed_10034" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10035" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10036" role="3EZMnx">
        <property role="3F0ifm" value="ForLoop: " />
      </node>
    </node>
  </node>

  <!-- Return Editor: return [value] -->
  <node concept="24kQdi" id="Ed_10037">
    <ref role="1XX52x" to="k8se:7kypvuIwDIC" resolve="Return" />
    <node concept="3EZMnI" id="Ed_10038" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10039" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10040" role="3EZMnx">
        <property role="3F0ifm" value="Return: " />
      </node>
    </node>
  </node>

  <!-- ExpressionStatement Editor: [expression] -->
  <node concept="24kQdi" id="Ed_10041">
    <ref role="1XX52x" to="k8se:7kypvuIwDJC" resolve="ExpressionStatement" />
    <node concept="3EZMnI" id="Ed_10042" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10043" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10044" role="3EZMnx">
        <property role="3F0ifm" value="ExpressionStatement: " />
      </node>
    </node>
  </node>

  <!-- BinaryOperation Editor: [left] [op] [right] -->
  <node concept="24kQdi" id="Ed_10045">
    <ref role="1XX52x" to="k8se:4ypvuIwCIC2" resolve="BinaryOperation" />
    <node concept="3EZMnI" id="Ed_10046" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10047" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10048" role="3EZMnx">
        <property role="3F0ifm" value="BinaryOperation: " />
      </node>
    </node>
  </node>

  <!-- UnaryOperation Editor: [op][operand] -->
  <node concept="24kQdi" id="Ed_10049">
    <ref role="1XX52x" to="k8se:7kypvuIwCJC" resolve="UnaryOperation" />
    <node concept="3EZMnI" id="Ed_10050" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10051" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10052" role="3EZMnx">
        <property role="3F0ifm" value="UnaryOperation: " />
      </node>
    </node>
  </node>

  <!-- FunctionCall Editor: [name]([args]) -->
  <node concept="24kQdi" id="Ed_10053">
    <ref role="1XX52x" to="k8se:7kypvuIwCLC" resolve="FunctionCall" />
    <node concept="3EZMnI" id="Ed_10054" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10055" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10056" role="3EZMnx">
        <property role="3F0ifm" value="FunctionCall: " />
      </node>
    </node>
  </node>

  <!-- VariableReference Editor: [name] -->
  <node concept="24kQdi" id="Ed_10057">
    <ref role="1XX52x" to="k8se:7kypvuIwCKC" resolve="VariableReference" />
    <node concept="3EZMnI" id="Ed_10058" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10059" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10060" role="3EZMnx">
        <property role="3F0ifm" value="VariableReference: " />
      </node>
    </node>
  </node>

  <!-- IntegerLiteral Editor: [value] -->
  <node concept="24kQdi" id="Ed_10061">
    <ref role="1XX52x" to="k8se:7kypvuIwCDC" resolve="IntegerLiteral" />
    <node concept="3EZMnI" id="Ed_10062" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10063" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10064" role="3EZMnx">
        <property role="3F0ifm" value="IntegerLiteral: " />
      </node>
    </node>
  </node>

  <!-- FloatLiteral Editor: [value] -->
  <node concept="24kQdi" id="Ed_10065">
    <ref role="1XX52x" to="k8se:7kypvuIwCEC" resolve="FloatLiteral" />
    <node concept="3EZMnI" id="Ed_10066" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10067" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10068" role="3EZMnx">
        <property role="3F0ifm" value="FloatLiteral: " />
      </node>
    </node>
  </node>

  <!-- StringLiteral Editor: "[value]" -->
  <node concept="24kQdi" id="Ed_10069">
    <ref role="1XX52x" to="k8se:7kypvuIwCFC" resolve="StringLiteral" />
    <node concept="3EZMnI" id="Ed_10070" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10071" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10072" role="3EZMnx">
        <property role="3F0ifm" value="StringLiteral: " />
      </node>
    </node>
  </node>

  <!-- BooleanLiteral Editor: [value] -->
  <node concept="24kQdi" id="Ed_10073">
    <ref role="1XX52x" to="k8se:7kypvuIwCGC" resolve="BooleanLiteral" />
    <node concept="3EZMnI" id="Ed_10074" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10075" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10076" role="3EZMnx">
        <property role="3F0ifm" value="BooleanLiteral: " />
      </node>
    </node>
  </node>

  <!-- NullLiteral Editor: null -->
  <node concept="24kQdi" id="Ed_10077">
    <ref role="1XX52x" to="k8se:7kypvuIwCHC" resolve="NullLiteral" />
    <node concept="3EZMnI" id="Ed_10078" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10079" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10080" role="3EZMnx">
        <property role="3F0ifm" value="NullLiteral: " />
      </node>
    </node>
  </node>

  <!-- ListLiteral Editor: [elements] -->
  <node concept="24kQdi" id="Ed_10081">
    <ref role="1XX52x" to="k8se:7kypvuIwCMC" resolve="ListLiteral" />
    <node concept="3EZMnI" id="Ed_10082" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10083" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10084" role="3EZMnx">
        <property role="3F0ifm" value="ListLiteral: " />
      </node>
    </node>
  </node>

  <!-- IndexAccess Editor: [target][[index]] -->
  <node concept="24kQdi" id="Ed_10085">
    <ref role="1XX52x" to="k8se:7kypvuIwCNC" resolve="IndexAccess" />
    <node concept="3EZMnI" id="Ed_10086" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10087" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10088" role="3EZMnx">
        <property role="3F0ifm" value="IndexAccess: " />
      </node>
    </node>
  </node>

  <!-- MemberAccess Editor: [target].[member] -->
  <node concept="24kQdi" id="Ed_10089">
    <ref role="1XX52x" to="k8se:7kypvuIwCOC" resolve="MemberAccess" />
    <node concept="3EZMnI" id="Ed_10090" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10091" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10092" role="3EZMnx">
        <property role="3F0ifm" value="MemberAccess: " />
      </node>
    </node>
  </node>

  <!-- PrimitiveType Editor: [kind] -->
  <node concept="24kQdi" id="Ed_10093">
    <ref role="1XX52x" to="k8se:7kypvuIwBCC" resolve="PrimitiveType" />
    <node concept="3EZMnI" id="Ed_10094" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10095" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10096" role="3EZMnx">
        <property role="3F0ifm" value="PrimitiveType: " />
      </node>
    </node>
  </node>

  <!-- ListType Editor: list[[elementType]] -->
  <node concept="24kQdi" id="Ed_10097">
    <ref role="1XX52x" to="k8se:7kypvuIwBDC" resolve="ListType" />
    <node concept="3EZMnI" id="Ed_10098" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10099" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10100" role="3EZMnx">
        <property role="3F0ifm" value="ListType: " />
      </node>
    </node>
  </node>

  <!-- SetType Editor: set[[elementType]] -->
  <node concept="24kQdi" id="Ed_10101">
    <ref role="1XX52x" to="k8se:7kypvuIwBGC" resolve="SetType" />
    <node concept="3EZMnI" id="Ed_10102" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10103" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10104" role="3EZMnx">
        <property role="3F0ifm" value="SetType: " />
      </node>
    </node>
  </node>

  <!-- MapType Editor: map[[keyType], [valueType]] -->
  <node concept="24kQdi" id="Ed_10105">
    <ref role="1XX52x" to="k8se:7kypvuIwBEC" resolve="MapType" />
    <node concept="3EZMnI" id="Ed_10106" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10107" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10108" role="3EZMnx">
        <property role="3F0ifm" value="MapType: " />
      </node>
    </node>
  </node>

  <!-- TupleType Editor: tuple[[elementTypes]] -->
  <node concept="24kQdi" id="Ed_10109">
    <ref role="1XX52x" to="k8se:7kypvuIwBHC" resolve="TupleType" />
    <node concept="3EZMnI" id="Ed_10110" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10111" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10112" role="3EZMnx">
        <property role="3F0ifm" value="TupleType: " />
      </node>
    </node>
  </node>

  <!-- ArrayType Editor: [elementType][[size]] -->
  <node concept="24kQdi" id="Ed_10113">
    <ref role="1XX52x" to="k8se:7kypvuIwBIC" resolve="ArrayType" />
    <node concept="3EZMnI" id="Ed_10114" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10115" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10116" role="3EZMnx">
        <property role="3F0ifm" value="ArrayType: " />
      </node>
    </node>
  </node>

  <!-- OptionalType Editor: optional[[innerType]] -->
  <node concept="24kQdi" id="Ed_10117">
    <ref role="1XX52x" to="k8se:7kypvuIwBFC" resolve="OptionalType" />
    <node concept="3EZMnI" id="Ed_10118" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10119" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10120" role="3EZMnx">
        <property role="3F0ifm" value="OptionalType: " />
      </node>
    </node>
  </node>

  <!-- CustomType Editor: [typeName] -->
  <node concept="24kQdi" id="Ed_10121">
    <ref role="1XX52x" to="k8se:7kypvuIwBJC" resolve="CustomType" />
    <node concept="3EZMnI" id="Ed_10122" role="2wV5jI">
      <node concept="2iRfu4" id="Ed_10123" role="2iSdaV" />
      <node concept="3F0ifn" id="Ed_10124" role="3EZMnx">
        <property role="3F0ifm" value="CustomType: " />
      </node>
    </node>
  </node>
</model>

