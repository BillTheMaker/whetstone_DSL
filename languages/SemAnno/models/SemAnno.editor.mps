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
</model>

