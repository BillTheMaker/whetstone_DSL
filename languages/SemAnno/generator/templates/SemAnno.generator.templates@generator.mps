<?xml version="1.0" encoding="UTF-8"?>
<model ref="r:c9548ba7-bc40-4265-85dc-04a05de5ae1e(SemAnno.generator.templates@generator)">
  <persistence version="9" />
  <languages>
    <use id="a452e1cf-6f8c-4bca-8447-fd4d6e4d0003" name="SemAnno" version="0" />
    <use id="b401a680-8325-4110-8fd3-84331ff25bef" name="jetbrains.mps.lang.generator" version="4" />
    <devkit ref="a2eb3a43-fcc2-4200-80dc-c60110c4862d(jetbrains.mps.devkit.templates)" />
  </languages>
  <imports>
    <import index="k8se" ref="r:e209a700-5f8c-468c-afcc-f38277628970(SemAnno.structure)" implicit="true" />
    <import index="tpck" ref="r:00000000-0000-4000-0000-011c89590288(jetbrains.mps.lang.core.structure)" implicit="true" />
  </imports>
  <registry>
    <language id="f3061a53-9226-4cc5-a443-f952ceaf5816" name="jetbrains.mps.baseLanguage">
      <concept id="1197027756228" name="jetbrains.mps.baseLanguage.structure.DotExpression" flags="nn" index="2OqwBi">
        <child id="1197027771414" name="operand" index="2Oq$k0" />
        <child id="1197027833540" name="operation" index="2OqNvi" />
      </concept>
      <concept id="1137021947720" name="jetbrains.mps.baseLanguage.structure.ConceptFunction" flags="in" index="2VMwT0">
        <child id="1137022507850" name="body" index="2VODD2" />
      </concept>
      <concept id="1070475926800" name="jetbrains.mps.baseLanguage.structure.StringLiteral" flags="nn" index="Xl_RD">
        <property id="1070475926801" name="value" index="Xl_RC" />
      </concept>
      <concept id="1068580123155" name="jetbrains.mps.baseLanguage.structure.ExpressionStatement" flags="nn" index="3clFbF">
        <child id="1068580123156" name="expression" index="3clFbG" />
      </concept>
      <concept id="1068580123136" name="jetbrains.mps.baseLanguage.structure.StatementList" flags="sn" stub="5293379017992965193" index="3clFbS">
        <child id="1068581517665" name="statement" index="3cqZAp" />
      </concept>
      <concept id="1068581242875" name="jetbrains.mps.baseLanguage.structure.PlusExpression" flags="nn" index="3cpWs3" />
      <concept id="1081773326031" name="jetbrains.mps.baseLanguage.structure.BinaryOperation" flags="nn" index="3uHJSO">
        <child id="1081773367579" name="rightExpression" index="3uHU7w" />
        <child id="1081773367580" name="leftExpression" index="3uHU7B" />
      </concept>
    </language>
    <language id="a452e1cf-6f8c-4bca-8447-fd4d6e4d0003" name="SemAnno">
      <concept id="806857647106076122" name="SemAnno.structure.TextLine" flags="ng" index="1r8ykr">
        <property id="806857647106076361" name="content" index="1r8yo8" />
      </concept>
      <concept id="806857647106076373" name="SemAnno.structure.PythonScript" flags="ng" index="1r8yok">
        <child id="806857647106076379" name="lines" index="1r8yoq" />
      </concept>
    </language>
    <language id="b401a680-8325-4110-8fd3-84331ff25bef" name="jetbrains.mps.lang.generator">
      <concept id="1114729360583" name="jetbrains.mps.lang.generator.structure.CopySrcListMacro" flags="ln" index="2b32R4">
        <child id="1168278589236" name="sourceNodesQuery" index="2P8S$" />
      </concept>
      <concept id="1095416546421" name="jetbrains.mps.lang.generator.structure.MappingConfiguration" flags="ig" index="bUwia">
        <child id="1167328349397" name="reductionMappingRule" index="3acgRq" />
        <child id="1167514678247" name="rootMappingRule" index="3lj3bC" />
      </concept>
      <concept id="1168559333462" name="jetbrains.mps.lang.generator.structure.TemplateDeclarationReference" flags="ln" index="j$656" />
      <concept id="1168619357332" name="jetbrains.mps.lang.generator.structure.RootTemplateAnnotation" flags="lg" index="n94m4">
        <reference id="1168619429071" name="applicableConcept" index="n9lRv" />
      </concept>
      <concept id="1095672379244" name="jetbrains.mps.lang.generator.structure.TemplateFragment" flags="ng" index="raruj" />
      <concept id="1722980698497626400" name="jetbrains.mps.lang.generator.structure.ITemplateCall" flags="ngI" index="v9R3L">
        <reference id="1722980698497626483" name="template" index="v9R2y" />
      </concept>
      <concept id="1167169188348" name="jetbrains.mps.lang.generator.structure.TemplateFunctionParameter_sourceNode" flags="nn" index="30H73N" />
      <concept id="1167169308231" name="jetbrains.mps.lang.generator.structure.BaseMappingRule" flags="ng" index="30H$t8">
        <reference id="1167169349424" name="applicableConcept" index="30HIoZ" />
      </concept>
      <concept id="1092059087312" name="jetbrains.mps.lang.generator.structure.TemplateDeclaration" flags="ig" index="13MO4I">
        <reference id="1168285871518" name="applicableConcept" index="3gUMe" />
        <child id="1092060348987" name="contentNode" index="13RCb5" />
      </concept>
      <concept id="1087833241328" name="jetbrains.mps.lang.generator.structure.PropertyMacro" flags="ln" index="17Uvod">
        <child id="1167756362303" name="propertyValueFunction" index="3zH0cK" />
      </concept>
      <concept id="1167327847730" name="jetbrains.mps.lang.generator.structure.Reduction_MappingRule" flags="lg" index="3aamgX">
        <child id="1169672767469" name="ruleConsequence" index="1lVwrX" />
      </concept>
      <concept id="1167514355419" name="jetbrains.mps.lang.generator.structure.Root_MappingRule" flags="lg" index="3lhOvk">
        <reference id="1167514355421" name="template" index="3lhOvi" />
      </concept>
      <concept id="1167756080639" name="jetbrains.mps.lang.generator.structure.PropertyMacro_GetPropertyValue" flags="in" index="3zFVjK" />
      <concept id="1167951910403" name="jetbrains.mps.lang.generator.structure.SourceSubstituteMacro_SourceNodesQuery" flags="in" index="3JmXsc" />
      <concept id="1118786554307" name="jetbrains.mps.lang.generator.structure.LoopMacro" flags="ln" index="1WS0z7">
        <child id="1167952069335" name="sourceNodesQuery" index="3Jn$fo" />
      </concept>
    </language>
    <language id="7866978e-a0f0-4cc7-81bc-4d213d9375e1" name="jetbrains.mps.lang.smodel">
      <concept id="1138056022639" name="jetbrains.mps.lang.smodel.structure.SPropertyAccess" flags="nn" index="3TrcHB">
        <reference id="1138056395725" name="property" index="3TsBF5" />
      </concept>
      <concept id="1138056282393" name="jetbrains.mps.lang.smodel.structure.SLinkListAccess" flags="nn" index="3Tsc0h">
        <reference id="1138056546658" name="link" index="3TtcxE" />
      </concept>
    </language>
    <language id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core">
      <concept id="1133920641626" name="jetbrains.mps.lang.core.structure.BaseConcept" flags="ng" index="2VYdi">
        <child id="5169995583184591170" name="smodelAttribute" index="lGtFl" />
      </concept>
      <concept id="3364660638048049750" name="jetbrains.mps.lang.core.structure.PropertyAttribute" flags="ng" index="A9Btg">
        <property id="1757699476691236117" name="name_DebugInfo" index="2qtEX9" />
        <property id="1341860900487648621" name="propertyId" index="P4ACc" />
      </concept>
      <concept id="1169194658468" name="jetbrains.mps.lang.core.structure.INamedConcept" flags="ngI" index="TrEIO">
        <property id="1169194664001" name="name" index="TrG5h" />
      </concept>
    </language>
  </registry>
  <node concept="bUwia" id="7_JEctSCWKH">
    <property role="TrG5h" value="main" />
    <node concept="3lhOvk" id="GMyc5g4G9C" role="3lj3bC">
      <ref role="30HIoZ" to="k8se:CisPcP0y__" resolve="Script" />
      <ref role="3lhOvi" node="GMyc5g4G9G" resolve="map_ScriptToPython" />
    </node>
    <node concept="3aamgX" id="GMyc5g4Jo4" role="3acgRq">
      <ref role="30HIoZ" to="k8se:CisPcOYEky" resolve="SemanticEntity" />
      <node concept="j$656" id="GMyc5g4Jo8" role="1lVwrX">
        <ref role="v9R2y" node="GMyc5g4Jo6" resolve="reduce_SemanticEntity" />
      </node>
    </node>
    <node concept="3aamgX" id="6HBlEl9FKAL" role="3acgRq">
      <ref role="30HIoZ" to="k8se:CisPcOYEk$" resolve="DataComponent" />
      <node concept="j$656" id="6HBlEl9Glsc" role="1lVwrX">
        <ref role="v9R2y" node="6HBlEl9Glsa" resolve="reduce_DataComponent" />
      </node>
    </node>
  </node>
  <node concept="1r8yok" id="GMyc5g4G9G">
    <property role="TrG5h" value="map_ScriptToPython" />
    <node concept="n94m4" id="GMyc5g4G9H" role="lGtFl">
      <ref role="n9lRv" to="k8se:CisPcP0y__" resolve="Script" />
    </node>
    <node concept="17Uvod" id="GMyc5g4G9I" role="lGtFl">
      <property role="2qtEX9" value="name" />
      <property role="P4ACc" value="ceab5195-25ea-4f22-9b92-103b95ca8c0c/1169194658468/1169194664001" />
      <node concept="3zFVjK" id="GMyc5g4G9J" role="3zH0cK">
        <node concept="3clFbS" id="GMyc5g4G9K" role="2VODD2">
          <node concept="3clFbF" id="GMyc5g4Gg1" role="3cqZAp">
            <node concept="2OqwBi" id="GMyc5g4G$q" role="3clFbG">
              <node concept="30H73N" id="GMyc5g4Gg0" role="2Oq$k0" />
              <node concept="3TrcHB" id="GMyc5g4IyS" role="2OqNvi">
                <ref role="3TsBF5" to="tpck:h0TrG11" resolve="name" />
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
    <node concept="1r8ykr" id="7dD_UEaMNHZ" role="1r8yoq">
      <node concept="1WS0z7" id="7dD_UEaMNK3" role="lGtFl">
        <node concept="3JmXsc" id="7dD_UEaMNK6" role="3Jn$fo">
          <node concept="3clFbS" id="7dD_UEaMNK7" role="2VODD2">
            <node concept="3clFbF" id="7dD_UEaMNKd" role="3cqZAp">
              <node concept="2OqwBi" id="7dD_UEaMNK8" role="3clFbG">
                <node concept="3Tsc0h" id="7dD_UEaMNKb" role="2OqNvi">
                  <ref role="3TtcxE" to="k8se:CisPcP0y_C" resolve="entities" />
                </node>
                <node concept="30H73N" id="7dD_UEaMNKc" role="2Oq$k0" />
              </node>
            </node>
          </node>
        </node>
      </node>
      <node concept="2b32R4" id="7dD_UEaMNZF" role="lGtFl">
        <node concept="3JmXsc" id="7dD_UEaMNZI" role="2P8S$">
          <node concept="3clFbS" id="7dD_UEaMNZJ" role="2VODD2">
            <node concept="3clFbF" id="7dD_UEaMNZP" role="3cqZAp">
              <node concept="2OqwBi" id="7dD_UEaMNZK" role="3clFbG">
                <node concept="3Tsc0h" id="7dD_UEaMNZN" role="2OqNvi">
                  <ref role="3TtcxE" to="k8se:CisPcOYEkA" resolve="components" />
                </node>
                <node concept="30H73N" id="7dD_UEaMNZO" role="2Oq$k0" />
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="13MO4I" id="GMyc5g4Jo6">
    <property role="TrG5h" value="reduce_SemanticEntity" />
    <ref role="3gUMe" to="k8se:CisPcOYEky" resolve="SemanticEntity" />
    <node concept="1r8ykr" id="GMyc5g4Joc" role="13RCb5">
      <property role="1r8yo8" value="$" />
      <node concept="raruj" id="7dD_UEaLBwE" role="lGtFl" />
      <node concept="17Uvod" id="7dD_UEaLDyp" role="lGtFl">
        <property role="2qtEX9" value="name" />
        <property role="P4ACc" value="a452e1cf-6f8c-4bca-8447-fd4d6e4d0003/806857647106076122/806857647106076361" />
        <node concept="3zFVjK" id="7dD_UEaLDyq" role="3zH0cK">
          <node concept="3clFbS" id="7dD_UEaLDyr" role="2VODD2">
            <node concept="3clFbF" id="7dD_UEaLDCG" role="3cqZAp">
              <node concept="3cpWs3" id="7dD_UEaLFnZ" role="3clFbG">
                <node concept="2OqwBi" id="7dD_UEaLFSU" role="3uHU7w">
                  <node concept="30H73N" id="7dD_UEaLFur" role="2Oq$k0" />
                  <node concept="3TrcHB" id="7dD_UEaLHQk" role="2OqNvi">
                    <ref role="3TsBF5" to="tpck:h0TrG11" resolve="name" />
                  </node>
                </node>
                <node concept="Xl_RD" id="7dD_UEaLDCF" role="3uHU7B">
                  <property role="Xl_RC" value=" # Entity: " />
                </node>
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="13MO4I" id="lVO1C_bvCC">
    <property role="TrG5h" value="reduce_DataComponent" />
    <ref role="3gUMe" to="k8se:CisPcOYEk$" resolve="DataComponent" />
    <node concept="2VYdi" id="lVO1C_bwsV" role="13RCb5" />
  </node>
</model>

