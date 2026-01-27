<?xml version="1.0" encoding="UTF-8"?>
<model ref="r:9d9f1195-38ea-441f-acf8-519a7a38131b(SemAnno.textGen)">
  <persistence version="9" />
  <languages>
    <use id="b83431fe-5c8f-40bc-8a36-65e25f4dd253" name="jetbrains.mps.lang.textGen" version="1" />
    <use id="68015e26-cc4d-49db-8715-b643faea1769" name="jetbrains.mps.lang.test.generator" version="0" />
    <use id="f3061a53-9226-4cc5-a443-f952ceaf5816" name="jetbrains.mps.baseLanguage" version="12" />
    <devkit ref="fa73d85a-ac7f-447b-846c-fcdc41caa600(jetbrains.mps.devkit.aspect.textgen)" />
    <devkit ref="fbc25dd2-5da4-483a-8b19-70928e1b62d7(jetbrains.mps.devkit.general-purpose)" />
    <devkit ref="2677cb18-f558-4e33-bc38-a5139cee06dc(jetbrains.mps.devkit.language-design)" />
  </languages>
  <imports>
    <import index="k8se" ref="r:e209a700-5f8c-468c-afcc-f38277628970(SemAnno.structure)" implicit="true" />
    <import index="tpck" ref="r:00000000-0000-4000-0000-011c89590288(jetbrains.mps.lang.core.structure)" implicit="true" />
  </imports>
  <registry>
    <language id="f3061a53-9226-4cc5-a443-f952ceaf5816" name="jetbrains.mps.baseLanguage">
      <concept id="1154032098014" name="jetbrains.mps.baseLanguage.structure.AbstractLoopStatement" flags="nn" index="2LF5Ji">
        <child id="1154032183016" name="body" index="2LFqv$" />
      </concept>
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
      <concept id="1068580123157" name="jetbrains.mps.baseLanguage.structure.Statement" flags="nn" index="3clFbH" />
      <concept id="1068580123136" name="jetbrains.mps.baseLanguage.structure.StatementList" flags="sn" stub="5293379017992965193" index="3clFbS">
        <child id="1068581517665" name="statement" index="3cqZAp" />
      </concept>
      <concept id="1068581242878" name="jetbrains.mps.baseLanguage.structure.ReturnStatement" flags="nn" index="3cpWs6">
        <child id="1068581517676" name="expression" index="3cqZAk" />
      </concept>
    </language>
    <language id="b83431fe-5c8f-40bc-8a36-65e25f4dd253" name="jetbrains.mps.lang.textGen">
      <concept id="8931911391946696733" name="jetbrains.mps.lang.textGen.structure.ExtensionDeclaration" flags="in" index="9MYSb" />
      <concept id="1237305208784" name="jetbrains.mps.lang.textGen.structure.NewLineAppendPart" flags="ng" index="l8MVK" />
      <concept id="1237305334312" name="jetbrains.mps.lang.textGen.structure.NodeAppendPart" flags="ng" index="l9hG8">
        <child id="1237305790512" name="value" index="lb14g" />
      </concept>
      <concept id="1237305557638" name="jetbrains.mps.lang.textGen.structure.ConstantStringAppendPart" flags="ng" index="la8eA">
        <property id="1237305576108" name="value" index="lacIc" />
      </concept>
      <concept id="1237306079178" name="jetbrains.mps.lang.textGen.structure.AppendOperation" flags="nn" index="lc7rE">
        <child id="1237306115446" name="part" index="lcghm" />
      </concept>
      <concept id="1233670071145" name="jetbrains.mps.lang.textGen.structure.ConceptTextGenDeclaration" flags="ig" index="WtQ9Q">
        <reference id="1233670257997" name="conceptDeclaration" index="WuzLi" />
        <child id="1233749296504" name="textGenBlock" index="11c4hB" />
        <child id="7991274449437422201" name="extension" index="33IsuW" />
      </concept>
      <concept id="1233748055915" name="jetbrains.mps.lang.textGen.structure.NodeParameter" flags="nn" index="117lpO" />
      <concept id="1233749247888" name="jetbrains.mps.lang.textGen.structure.GenerateTextDeclaration" flags="in" index="11bSqf" />
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
      <concept id="1169194658468" name="jetbrains.mps.lang.core.structure.INamedConcept" flags="ngI" index="TrEIO">
        <property id="1169194664001" name="name" index="TrG5h" />
      </concept>
    </language>
    <language id="83888646-71ce-4f1c-9c53-c54016f6ad4f" name="jetbrains.mps.baseLanguage.collections">
      <concept id="1153943597977" name="jetbrains.mps.baseLanguage.collections.structure.ForEachStatement" flags="nn" index="2Gpval">
        <child id="1153944400369" name="variable" index="2Gsz3X" />
        <child id="1153944424730" name="inputSequence" index="2GsD0m" />
      </concept>
      <concept id="1153944193378" name="jetbrains.mps.baseLanguage.collections.structure.ForEachVariable" flags="nr" index="2GrKxI" />
      <concept id="1153944233411" name="jetbrains.mps.baseLanguage.collections.structure.ForEachVariableReference" flags="nn" index="2GrUjf">
        <reference id="1153944258490" name="variable" index="2Gs0qQ" />
      </concept>
    </language>
  </registry>
  <node concept="WtQ9Q" id="CisPcOZtYe">
    <ref role="WuzLi" to="k8se:CisPcOYEky" resolve="SemanticEntity" />
    <node concept="11bSqf" id="CisPcOZujo" role="11c4hB">
      <node concept="3clFbS" id="CisPcOZujp" role="2VODD2">
        <node concept="lc7rE" id="CisPcOZxfc" role="3cqZAp">
          <node concept="la8eA" id="CisPcOZxfE" role="lcghm">
            <property role="lacIc" value="# Entity: " />
          </node>
        </node>
        <node concept="lc7rE" id="CisPcOZxhW" role="3cqZAp">
          <node concept="l9hG8" id="CisPcOZxiq" role="lcghm">
            <node concept="2OqwBi" id="CisPcOZxtV" role="lb14g">
              <node concept="117lpO" id="CisPcOZxiX" role="2Oq$k0" />
              <node concept="3TrcHB" id="CisPcOZzzg" role="2OqNvi">
                <ref role="3TsBF5" to="tpck:h0TrG11" resolve="name" />
              </node>
            </node>
          </node>
        </node>
        <node concept="lc7rE" id="CisPcOZzFJ" role="3cqZAp">
          <node concept="l8MVK" id="CisPcOZzG_" role="lcghm" />
        </node>
        <node concept="3clFbH" id="CisPcOZzH1" role="3cqZAp" />
        <node concept="2Gpval" id="CisPcOZzLf" role="3cqZAp">
          <node concept="2GrKxI" id="CisPcOZzLh" role="2Gsz3X">
            <property role="TrG5h" value="component" />
          </node>
          <node concept="2OqwBi" id="CisPcOZzYN" role="2GsD0m">
            <node concept="117lpO" id="CisPcOZzO9" role="2Oq$k0" />
            <node concept="3Tsc0h" id="CisPcOZ$uT" role="2OqNvi">
              <ref role="3TtcxE" to="k8se:CisPcOYEkA" resolve="components" />
            </node>
          </node>
          <node concept="3clFbS" id="CisPcOZzLl" role="2LFqv$">
            <node concept="lc7rE" id="CisPcOZ$zv" role="3cqZAp">
              <node concept="l9hG8" id="CisPcOZ_hL" role="lcghm">
                <node concept="2GrUjf" id="CisPcOZ_ij" role="lb14g">
                  <ref role="2Gs0qQ" node="CisPcOZzLh" resolve="component" />
                </node>
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="CisPcOZ$zX">
    <ref role="WuzLi" to="k8se:CisPcOZ8mj" resolve="StringList" />
    <node concept="11bSqf" id="CisPcOZ$$0" role="11c4hB">
      <node concept="3clFbS" id="CisPcOZ$$1" role="2VODD2">
        <node concept="lc7rE" id="CisPcOZ$$q" role="3cqZAp">
          <node concept="la8eA" id="CisPcOZ$$P" role="lcghm">
            <property role="lacIc" value="data_list = \&quot;" />
          </node>
        </node>
        <node concept="lc7rE" id="CisPcOZ$Ca" role="3cqZAp">
          <node concept="l9hG8" id="CisPcOZ$CB" role="lcghm">
            <node concept="2OqwBi" id="CisPcOZ$O4" role="lb14g">
              <node concept="117lpO" id="CisPcOZ$D9" role="2Oq$k0" />
              <node concept="3TrcHB" id="CisPcOZ_4l" role="2OqNvi">
                <ref role="3TsBF5" to="k8se:CisPcOZ8mk" resolve="values" />
              </node>
            </node>
          </node>
        </node>
        <node concept="lc7rE" id="CisPcOZ_7N" role="3cqZAp">
          <node concept="la8eA" id="CisPcOZ_9K" role="lcghm">
            <property role="lacIc" value="\&quot;.split(\&quot;, \&quot;)" />
          </node>
        </node>
        <node concept="lc7rE" id="CisPcOZ_gy" role="3cqZAp">
          <node concept="l8MVK" id="CisPcOZ_hm" role="lcghm" />
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="CisPcP0a0Q">
    <ref role="WuzLi" to="k8se:CisPcOZ8mz" resolve="Iterator" />
    <node concept="11bSqf" id="CisPcP0a0S" role="11c4hB">
      <node concept="3clFbS" id="CisPcP0a0T" role="2VODD2">
        <node concept="lc7rE" id="CisPcP0a1j" role="3cqZAp">
          <node concept="la8eA" id="CisPcP0a1H" role="lcghm">
            <property role="lacIc" value="for item in data_list:" />
          </node>
        </node>
        <node concept="lc7rE" id="CisPcP0a4E" role="3cqZAp">
          <node concept="l8MVK" id="CisPcP0a57" role="lcghm" />
        </node>
        <node concept="lc7rE" id="CisPcP0a5R" role="3cqZAp">
          <node concept="la8eA" id="CisPcP0a6k" role="lcghm">
            <property role="lacIc" value="    print(item)" />
          </node>
        </node>
        <node concept="lc7rE" id="CisPcP0a8U" role="3cqZAp">
          <node concept="l8MVK" id="CisPcP0a9m" role="lcghm" />
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="CisPcP0yAa">
    <ref role="WuzLi" to="k8se:CisPcP0y__" resolve="Script" />
    <node concept="11bSqf" id="CisPcP0yAc" role="11c4hB">
      <node concept="3clFbS" id="CisPcP0yAd" role="2VODD2">
        <node concept="lc7rE" id="CisPcP0yAB" role="3cqZAp">
          <node concept="la8eA" id="CisPcP0yB2" role="lcghm">
            <property role="lacIc" value="# Whetstone Script: " />
          </node>
        </node>
        <node concept="lc7rE" id="CisPcP0yEI" role="3cqZAp">
          <node concept="l9hG8" id="CisPcP0yFa" role="lcghm">
            <node concept="2OqwBi" id="CisPcP0yQC" role="lb14g">
              <node concept="117lpO" id="CisPcP0yFF" role="2Oq$k0" />
              <node concept="3TrcHB" id="CisPcP0z5K" role="2OqNvi">
                <ref role="3TsBF5" to="tpck:h0TrG11" resolve="name" />
              </node>
            </node>
          </node>
        </node>
        <node concept="lc7rE" id="CisPcP0zbd" role="3cqZAp">
          <node concept="l8MVK" id="CisPcP0zc1" role="lcghm" />
        </node>
        <node concept="lc7rE" id="CisPcP0zda" role="3cqZAp">
          <node concept="l8MVK" id="CisPcP0zdY" role="lcghm" />
        </node>
        <node concept="3clFbH" id="CisPcP0zep" role="3cqZAp" />
        <node concept="2Gpval" id="CisPcP0zh8" role="3cqZAp">
          <node concept="2GrKxI" id="CisPcP0zha" role="2Gsz3X">
            <property role="TrG5h" value="entity" />
          </node>
          <node concept="2OqwBi" id="CisPcP0ztN" role="2GsD0m">
            <node concept="117lpO" id="CisPcP0zja" role="2Oq$k0" />
            <node concept="3Tsc0h" id="CisPcP0$SI" role="2OqNvi">
              <ref role="3TtcxE" to="k8se:CisPcP0y_C" resolve="entities" />
            </node>
          </node>
          <node concept="3clFbS" id="CisPcP0zhe" role="2LFqv$">
            <node concept="lc7rE" id="CisPcP0zYs" role="3cqZAp">
              <node concept="l9hG8" id="CisPcP0zYR" role="lcghm">
                <node concept="2GrUjf" id="CisPcP0zZo" role="lb14g">
                  <ref role="2Gs0qQ" node="CisPcP0zha" resolve="entity" />
                </node>
              </node>
            </node>
            <node concept="lc7rE" id="CisPcP0$0f" role="3cqZAp">
              <node concept="l8MVK" id="CisPcP0$0F" role="lcghm" />
            </node>
          </node>
        </node>
      </node>
    </node>
    <node concept="9MYSb" id="CisPcP0$86" role="33IsuW">
      <node concept="3clFbS" id="CisPcP0$87" role="2VODD2">
        <node concept="3cpWs6" id="CisPcP0_9i" role="3cqZAp">
          <node concept="Xl_RD" id="CisPcP0$jr" role="3cqZAk">
            <property role="Xl_RC" value="py" />
          </node>
        </node>
      </node>
    </node>
  </node>
</model>

