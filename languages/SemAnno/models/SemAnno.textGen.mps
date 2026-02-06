<?xml version="1.0" encoding="UTF-8"?>
<model ref="r:9d9f1195-38ea-441f-acf8-519a7a38131b(SemAnno.textGen)">
  <persistence version="9" />
  <languages>
    <use id="b83431fe-5c8f-40bc-8a36-65e25f4dd253" name="jetbrains.mps.lang.textGen" version="1" />
    <use id="68015e26-cc4d-49db-8715-b643faea1769" name="jetbrains.mps.lang.test.generator" version="0" />
    <use id="f3061a53-9226-4cc5-a443-f952ceaf5816" name="jetbrains.mps.baseLanguage" version="12" />
    <use id="7866978e-a0f0-4cc7-81bc-4d213d9375e1" name="jetbrains.mps.lang.smodel" version="19" />
    <devkit ref="fa73d85a-ac7f-447b-846c-fcdc41caa600(jetbrains.mps.devkit.aspect.textgen)" />
    <devkit ref="fbc25dd2-5da4-483a-8b19-70928e1b62d7(jetbrains.mps.devkit.general-purpose)" />
    <devkit ref="2677cb18-f558-4e33-bc38-a5139cee06dc(jetbrains.mps.devkit.language-design)" />
  </languages>
  <imports>
    <import index="cttk" ref="r:5ff047e0-2953-4750-806a-bdc16824aa89(jetbrains.mps.smodel)" />
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
      <concept id="1068580123155" name="jetbrains.mps.baseLanguage.structure.ExpressionStatement" flags="nn" index="3clFbF">
        <child id="1068580123156" name="expression" index="3clFbG" />
      </concept>
      <concept id="1068580123157" name="jetbrains.mps.baseLanguage.structure.Statement" flags="nn" index="3clFbH" />
      <concept id="1068580123159" name="jetbrains.mps.baseLanguage.structure.IfStatement" flags="nn" index="3clFbJ">
        <child id="1068580123160" name="condition" index="3clFbw" />
        <child id="1068580123161" name="ifTrue" index="3clFbx" />
      </concept>
      <concept id="1068580123136" name="jetbrains.mps.baseLanguage.structure.StatementList" flags="sn" stub="5293379017992965193" index="3clFbS">
        <child id="1068581517665" name="statement" index="3cqZAp" />
      </concept>
      <concept id="1081773326031" name="jetbrains.mps.baseLanguage.structure.BinaryOperation" flags="nn" index="3uHJSO">
        <child id="1081773367579" name="rightExpression" index="3uHU7w" />
        <child id="1081773367580" name="leftExpression" index="3uHU7B" />
      </concept>
      <concept id="1073239437375" name="jetbrains.mps.baseLanguage.structure.NotEqualsExpression" flags="nn" index="3y3z36" />
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
      <concept id="1138056143562" name="jetbrains.mps.lang.smodel.structure.SLinkAccess" flags="nn" index="3TrEf2">
        <reference id="1138056516764" name="link" index="3Tt5mk" />
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
  <node concept="WtQ9Q" id="7dD_UEaLI7H">
    <ref role="WuzLi" to="k8se:GMyc5g4aRq" resolve="TextLine" />
    <node concept="11bSqf" id="7dD_UEaLI7J" role="11c4hB">
      <node concept="3clFbS" id="7dD_UEaLI7K" role="2VODD2">
        <node concept="lc7rE" id="7dD_UEaLJAO" role="3cqZAp">
          <node concept="l9hG8" id="7dD_UEaLJCb" role="lcghm">
            <node concept="2OqwBi" id="7dD_UEaMk9$" role="lb14g">
              <node concept="117lpO" id="7dD_UEaMjZr" role="2Oq$k0" />
              <node concept="3TrcHB" id="7dD_UEaMkS0" role="2OqNvi">
                <ref role="3TsBF5" to="k8se:GMyc5g4aV9" resolve="content" />
              </node>
            </node>
          </node>
        </node>
        <node concept="lc7rE" id="7dD_UEaLIUG" role="3cqZAp">
          <node concept="l8MVK" id="7dD_UEaLIVk" role="lcghm" />
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="7dD_UEaMulq">
    <ref role="WuzLi" to="k8se:GMyc5g4aVl" resolve="PythonScript" />
    <node concept="9MYSb" id="7dD_UEaMuls" role="33IsuW">
      <node concept="3clFbS" id="7dD_UEaMult" role="2VODD2">
        <node concept="3clFbF" id="7dD_UEaMurL" role="3cqZAp">
          <node concept="Xl_RD" id="7dD_UEaMurK" role="3clFbG">
            <property role="Xl_RC" value="py" />
          </node>
        </node>
      </node>
    </node>
    <node concept="11bSqf" id="7dD_UEaMu$G" role="11c4hB">
      <node concept="3clFbS" id="7dD_UEaMu$H" role="2VODD2">
        <node concept="2Gpval" id="7dD_UEaMu_I" role="3cqZAp">
          <node concept="2GrKxI" id="7dD_UEaMu_J" role="2Gsz3X">
            <property role="TrG5h" value="line" />
          </node>
          <node concept="2OqwBi" id="7dD_UEaMuLu" role="2GsD0m">
            <node concept="117lpO" id="7dD_UEaMuAP" role="2Oq$k0" />
            <node concept="3Tsc0h" id="7dD_UEaMvcV" role="2OqNvi">
              <ref role="3TtcxE" to="k8se:GMyc5g4aVr" resolve="lines" />
            </node>
          </node>
          <node concept="3clFbS" id="7dD_UEaMu_L" role="2LFqv$">
            <node concept="lc7rE" id="7dD_UEaMvh8" role="3cqZAp">
              <node concept="l9hG8" id="7dD_UEaMvhz" role="lcghm">
                <node concept="2GrUjf" id="7dD_UEaMvi4" role="lb14g">
                  <ref role="2Gs0qQ" node="7dD_UEaMu_J" resolve="line" />
                </node>
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20007">
    <ref role="WuzLi" to="k8se:7kypvuIwBIC" resolve="ArrayType" />
    <node concept="11bSqf" id="TG_20008" role="11c4hB">
      <node concept="3clFbS" id="TG_20009" role="2VODD2">
        <node concept="lc7rE" id="TG_20010" role="3cqZAp">
          <node concept="la8eA" id="TG_20011" role="lcghm">
            <property role="lacIc" value="[elementType][[size]]" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20012">
    <ref role="WuzLi" to="k8se:7kypvuIwDEC" resolve="Assignment" />
    <node concept="11bSqf" id="TG_20013" role="11c4hB">
      <node concept="3clFbS" id="TG_20014" role="2VODD2">
        <node concept="lc7rE" id="FTG_A001" role="3cqZAp">
          <node concept="l9hG8" id="FTG_A002" role="lcghm">
            <node concept="2OqwBi" id="FTG_A003" role="lb14g">
              <node concept="117lpO" id="FTG_A004" role="2Oq$k0" />
              <node concept="3TrcHB" id="FTG_A005" role="2OqNvi">
                <ref role="3TsBF5" to="tpck:h0TrG11" resolve="name" />
              </node>
            </node>
          </node>
        </node>
        <node concept="lc7rE" id="FTG_A006" role="3cqZAp">
          <node concept="la8eA" id="FTG_A007" role="lcghm">
            <property role="lacIc" value=" = " />
          </node>
        </node>
        <node concept="lc7rE" id="FTG2_A001" role="3cqZAp">
          <node concept="l9hG8" id="FTG2_A002" role="lcghm">
            <node concept="2OqwBi" id="FTG2_A003" role="lb14g">
              <node concept="117lpO" id="FTG2_A004" role="2Oq$k0" />
              <node concept="3TrEf2" id="FTG2_A005" role="2OqNvi">
                <ref role="3Tt5mk" to="k8se:7kypvuIwDEE" resolve="value" />
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20017">
    <ref role="WuzLi" to="k8se:4ypvuIwCIC2" resolve="BinaryOperation" />
    <node concept="11bSqf" id="TG_20018" role="11c4hB">
      <node concept="3clFbS" id="TG_20019" role="2VODD2">
        <node concept="lc7rE" id="FTG_BO001" role="3cqZAp">
          <node concept="l9hG8" id="FTG_BO002" role="lcghm">
            <node concept="2OqwBi" id="FTG2_BO001" role="lb14g">
              <node concept="117lpO" id="FTG2_BO002" role="2Oq$k0" />
              <node concept="3TrEf2" id="FTG2_BO003" role="2OqNvi">
                <ref role="3Tt5mk" to="k8se:7kypvuIwCIE" resolve="left" />
              </node>
            </node>
          </node>
        </node>
        <node concept="lc7rE" id="FTG2_BO004" role="3cqZAp">
          <node concept="la8eA" id="FTG2_BO005" role="lcghm">
            <property role="lacIc" value=" " />
          </node>
        </node>
        <node concept="lc7rE" id="FTG_BO008" role="3cqZAp">
          <node concept="l9hG8" id="FTG_BO009" role="lcghm">
            <node concept="2OqwBi" id="FTG_BO010" role="lb14g">
              <node concept="117lpO" id="FTG_BO011" role="2Oq$k0" />
              <node concept="3TrcHB" id="FTG_BO012" role="2OqNvi">
                <ref role="3TsBF5" to="tpck:h0TrG11" resolve="name" />
              </node>
            </node>
          </node>
        </node>
        <node concept="lc7rE" id="FTG2_BO006" role="3cqZAp">
          <node concept="la8eA" id="FTG2_BO007" role="lcghm">
            <property role="lacIc" value=" " />
          </node>
        </node>
        <node concept="lc7rE" id="FTG_BO013" role="3cqZAp">
          <node concept="l9hG8" id="FTG_BO014" role="lcghm">
            <node concept="2OqwBi" id="FTG2_BO008" role="lb14g">
              <node concept="117lpO" id="FTG2_BO009" role="2Oq$k0" />
              <node concept="3TrEf2" id="FTG2_BO010" role="2OqNvi">
                <ref role="3Tt5mk" to="k8se:7kypvuIwCIF" resolve="right" />
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20022">
    <ref role="WuzLi" to="k8se:7kypvuIwDDD" resolve="Block" />
    <node concept="11bSqf" id="TG_20023" role="11c4hB">
      <node concept="3clFbS" id="TG_20024" role="2VODD2">
        <node concept="2Gpval" id="FTG_BL001" role="3cqZAp">
          <node concept="2GrKxI" id="FTG_BL002" role="2Gsz3X">
            <property role="TrG5h" value="stmt" />
          </node>
          <node concept="2OqwBi" id="FTG_BL003" role="2GsD0m">
            <node concept="117lpO" id="FTG_BL004" role="2Oq$k0" />
            <node concept="3Tsc0h" id="FTG_BL005" role="2OqNvi">
              <ref role="3TtcxE" to="k8se:4ypvuIwDDD2" resolve="statements" />
            </node>
          </node>
          <node concept="3clFbS" id="FTG_BL006" role="2LFqv$">
            <node concept="lc7rE" id="FTG2_BL001" role="3cqZAp">
              <node concept="la8eA" id="FTG2_BL002" role="lcghm">
                <property role="lacIc" value="    " />
              </node>
            </node>
            <node concept="lc7rE" id="FTG_BL007" role="3cqZAp">
              <node concept="l9hG8" id="FTG_BL008" role="lcghm">
                <node concept="2GrUjf" id="FTG_BL009" role="lb14g">
                  <ref role="2Gs0qQ" node="FTG_BL002" resolve="stmt" />
                </node>
              </node>
            </node>
            <node concept="lc7rE" id="FTG_BL010" role="3cqZAp">
              <node concept="l8MVK" id="FTG_BL011" role="lcghm" />
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20027">
    <ref role="WuzLi" to="k8se:7kypvuIwCGC" resolve="BooleanLiteral" />
    <node concept="11bSqf" id="TG_20028" role="11c4hB">
      <node concept="3clFbS" id="TG_20029" role="2VODD2">
        <node concept="lc7rE" id="TG_20030" role="3cqZAp">
          <node concept="la8eA" id="TG_20031" role="lcghm">
            <property role="lacIc" value="[value]" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20032">
    <ref role="WuzLi" to="k8se:7kypvuIwBJC" resolve="CustomType" />
    <node concept="11bSqf" id="TG_20033" role="11c4hB">
      <node concept="3clFbS" id="TG_20034" role="2VODD2">
        <node concept="lc7rE" id="TG_20035" role="3cqZAp">
          <node concept="la8eA" id="TG_20036" role="lcghm">
            <property role="lacIc" value="[typeName]" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20037">
    <ref role="WuzLi" to="k8se:7kypvuIwFCD" resolve="DerefStrategy" />
    <node concept="11bSqf" id="TG_20038" role="11c4hB">
      <node concept="3clFbS" id="TG_20039" role="2VODD2">
        <node concept="lc7rE" id="TG_20040" role="3cqZAp">
          <node concept="la8eA" id="TG_20041" role="lcghm">
            <property role="lacIc" value="@deref([strategy])" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20048">
    <ref role="WuzLi" to="k8se:7kypvuIwDJC" resolve="ExpressionStatement" />
    <node concept="11bSqf" id="TG_20049" role="11c4hB">
      <node concept="3clFbS" id="TG_20050" role="2VODD2">
        <node concept="lc7rE" id="FTG_ES001" role="3cqZAp">
          <node concept="l9hG8" id="FTG_ES002" role="lcghm">
            <node concept="2OqwBi" id="FTG2_ES001" role="lb14g">
              <node concept="117lpO" id="FTG2_ES002" role="2Oq$k0" />
              <node concept="3TrEf2" id="FTG2_ES003" role="2OqNvi">
                <ref role="3Tt5mk" to="k8se:7kypvuIwDJD" resolve="expression" />
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20053">
    <ref role="WuzLi" to="k8se:7kypvuIwCEC" resolve="FloatLiteral" />
    <node concept="11bSqf" id="TG_20054" role="11c4hB">
      <node concept="3clFbS" id="TG_20055" role="2VODD2">
        <node concept="lc7rE" id="TG_20056" role="3cqZAp">
          <node concept="la8eA" id="TG_20057" role="lcghm">
            <property role="lacIc" value="[value]" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20058">
    <ref role="WuzLi" to="k8se:7kypvuIwDHC" resolve="ForLoop" />
    <node concept="11bSqf" id="TG_20059" role="11c4hB">
      <node concept="3clFbS" id="TG_20060" role="2VODD2">
        <node concept="lc7rE" id="TG_20061" role="3cqZAp">
          <node concept="la8eA" id="TG_20062" role="lcghm">
            <property role="lacIc" value="for [iteratorName] in [iterable]:&#10;[body]&#10;" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20063">
    <ref role="WuzLi" to="k8se:7kypvuIwEDC" resolve="Function" />
    <node concept="11bSqf" id="TG_20064" role="11c4hB">
      <node concept="3clFbS" id="TG_20065" role="2VODD2">
        <node concept="lc7rE" id="FTG_F001" role="3cqZAp">
          <node concept="la8eA" id="FTG_F002" role="lcghm">
            <property role="lacIc" value="def " />
          </node>
        </node>
        <node concept="lc7rE" id="FTG_F003" role="3cqZAp">
          <node concept="l9hG8" id="FTG_F004" role="lcghm">
            <node concept="2OqwBi" id="FTG_F005" role="lb14g">
              <node concept="117lpO" id="FTG_F006" role="2Oq$k0" />
              <node concept="3TrcHB" id="FTG_F007" role="2OqNvi">
                <ref role="3TsBF5" to="tpck:h0TrG11" resolve="name" />
              </node>
            </node>
          </node>
        </node>
        <node concept="lc7rE" id="FTG_F008" role="3cqZAp">
          <node concept="la8eA" id="FTG_F009" role="lcghm">
            <property role="lacIc" value="(" />
          </node>
        </node>
        <node concept="2Gpval" id="FTG_F010" role="3cqZAp">
          <node concept="2GrKxI" id="FTG_F011" role="2Gsz3X">
            <property role="TrG5h" value="param" />
          </node>
          <node concept="2OqwBi" id="FTG_F012" role="2GsD0m">
            <node concept="117lpO" id="FTG_F013" role="2Oq$k0" />
            <node concept="3Tsc0h" id="FTG_F014" role="2OqNvi">
              <ref role="3TtcxE" to="k8se:7kypvuIwEDI" resolve="parameters" />
            </node>
          </node>
          <node concept="3clFbS" id="FTG_F015" role="2LFqv$">
            <node concept="lc7rE" id="FTG_F016" role="3cqZAp">
              <node concept="l9hG8" id="FTG_F017" role="lcghm">
                <node concept="2GrUjf" id="FTG_F018" role="lb14g">
                  <ref role="2Gs0qQ" node="FTG_F011" resolve="param" />
                </node>
              </node>
            </node>
            <node concept="lc7rE" id="FTG_F019" role="3cqZAp">
              <node concept="la8eA" id="FTG_F020" role="lcghm">
                <property role="lacIc" value=", " />
              </node>
            </node>
          </node>
        </node>
        <node concept="lc7rE" id="FTG_F021" role="3cqZAp">
          <node concept="la8eA" id="FTG_F022" role="lcghm">
            <property role="lacIc" value="):" />
          </node>
        </node>
        <node concept="lc7rE" id="FTG_F023" role="3cqZAp">
          <node concept="l8MVK" id="FTG_F024" role="lcghm" />
        </node>
        <node concept="2Gpval" id="FTG_F025" role="3cqZAp">
          <node concept="2GrKxI" id="FTG_F026" role="2Gsz3X">
            <property role="TrG5h" value="stmt" />
          </node>
          <node concept="2OqwBi" id="FTG_F027" role="2GsD0m">
            <node concept="117lpO" id="FTG_F028" role="2Oq$k0" />
            <node concept="3Tsc0h" id="FTG_F029" role="2OqNvi">
              <ref role="3TtcxE" to="k8se:7kypvuIwEDH" resolve="body" />
            </node>
          </node>
          <node concept="3clFbS" id="FTG_F030" role="2LFqv$">
            <node concept="lc7rE" id="FTG_F031" role="3cqZAp">
              <node concept="la8eA" id="FTG_F032" role="lcghm">
                <property role="lacIc" value="    " />
              </node>
            </node>
            <node concept="lc7rE" id="FTG_F033" role="3cqZAp">
              <node concept="l9hG8" id="FTG_F034" role="lcghm">
                <node concept="2GrUjf" id="FTG_F035" role="lb14g">
                  <ref role="2Gs0qQ" node="FTG_F026" resolve="stmt" />
                </node>
              </node>
            </node>
            <node concept="lc7rE" id="FTG_F036" role="3cqZAp">
              <node concept="l8MVK" id="FTG_F037" role="lcghm" />
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20068">
    <ref role="WuzLi" to="k8se:7kypvuIwCLC" resolve="FunctionCall" />
    <node concept="11bSqf" id="TG_20069" role="11c4hB">
      <node concept="3clFbS" id="TG_20070" role="2VODD2">
        <node concept="lc7rE" id="TG_20071" role="3cqZAp">
          <node concept="la8eA" id="TG_20072" role="lcghm">
            <property role="lacIc" value="[functionName]([arguments])" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20073">
    <ref role="WuzLi" to="k8se:7kypvuIwDFC" resolve="IfStatement" />
    <node concept="11bSqf" id="TG_20074" role="11c4hB">
      <node concept="3clFbS" id="TG_20075" role="2VODD2">
        <node concept="lc7rE" id="TG_20076" role="3cqZAp">
          <node concept="la8eA" id="TG_20077" role="lcghm">
            <property role="lacIc" value="if [condition]:&#10;[thenBranch]&#10;else:&#10;[elseBranch]&#10;" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20078">
    <ref role="WuzLi" to="k8se:7kypvuIwCNC" resolve="IndexAccess" />
    <node concept="11bSqf" id="TG_20079" role="11c4hB">
      <node concept="3clFbS" id="TG_20080" role="2VODD2">
        <node concept="lc7rE" id="TG_20081" role="3cqZAp">
          <node concept="la8eA" id="TG_20082" role="lcghm">
            <property role="lacIc" value="[target][[index]]" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20083">
    <ref role="WuzLi" to="k8se:7kypvuIwCDC" resolve="IntegerLiteral" />
    <node concept="11bSqf" id="TG_20084" role="11c4hB">
      <node concept="3clFbS" id="TG_20085" role="2VODD2">
        <node concept="lc7rE" id="FTG_IL001" role="3cqZAp">
          <node concept="la8eA" id="FTG_IL002" role="lcghm">
            <property role="lacIc" value="&lt;int&gt;" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20088">
    <ref role="WuzLi" to="k8se:7kypvuIwFED" resolve="LangSpecific" />
    <node concept="11bSqf" id="TG_20089" role="11c4hB">
      <node concept="3clFbS" id="TG_20090" role="2VODD2">
        <node concept="lc7rE" id="TG_20091" role="3cqZAp">
          <node concept="la8eA" id="TG_20092" role="lcghm">
            <property role="lacIc" value="@lang([language]:[idiomType])" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20093">
    <ref role="WuzLi" to="k8se:7kypvuIwCMC" resolve="ListLiteral" />
    <node concept="11bSqf" id="TG_20094" role="11c4hB">
      <node concept="3clFbS" id="TG_20095" role="2VODD2">
        <node concept="lc7rE" id="TG_20096" role="3cqZAp">
          <node concept="la8eA" id="TG_20097" role="lcghm">
            <property role="lacIc" value="[elements]" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20098">
    <ref role="WuzLi" to="k8se:7kypvuIwBDC" resolve="ListType" />
    <node concept="11bSqf" id="TG_20099" role="11c4hB">
      <node concept="3clFbS" id="TG_20100" role="2VODD2">
        <node concept="lc7rE" id="TG_20101" role="3cqZAp">
          <node concept="la8eA" id="TG_20102" role="lcghm">
            <property role="lacIc" value="list[[elementType]]" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20103">
    <ref role="WuzLi" to="k8se:7kypvuIwBEC" resolve="MapType" />
    <node concept="11bSqf" id="TG_20104" role="11c4hB">
      <node concept="3clFbS" id="TG_20105" role="2VODD2">
        <node concept="lc7rE" id="TG_20106" role="3cqZAp">
          <node concept="la8eA" id="TG_20107" role="lcghm">
            <property role="lacIc" value="map[[keyType], [valueType]]" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20108">
    <ref role="WuzLi" to="k8se:7kypvuIwCOC" resolve="MemberAccess" />
    <node concept="11bSqf" id="TG_20109" role="11c4hB">
      <node concept="3clFbS" id="TG_20110" role="2VODD2">
        <node concept="lc7rE" id="TG_20111" role="3cqZAp">
          <node concept="la8eA" id="TG_20112" role="lcghm">
            <property role="lacIc" value="[target].[memberName]" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20113">
    <ref role="WuzLi" to="k8se:7kypvuIwECC" resolve="Module" />
    <node concept="9MYSb" id="FTG_MEXT1" role="33IsuW">
      <node concept="3clFbS" id="FTG_MEXT2" role="2VODD2">
        <node concept="3clFbF" id="FTG_MEXT3" role="3cqZAp">
          <node concept="Xl_RD" id="FTG_MEXT4" role="3clFbG">
            <property role="Xl_RC" value="txt" />
          </node>
        </node>
      </node>
    </node>
    <node concept="11bSqf" id="TG_20114" role="11c4hB">
      <node concept="3clFbS" id="TG_20115" role="2VODD2">
        <node concept="3clFbJ" id="IF_PY001" role="3cqZAp">
          <node concept="3clFbS" id="IF_PY002" role="3clFbx">
            <node concept="lc7rE" id="FTG_M001" role="3cqZAp">
              <node concept="la8eA" id="FTG_M002" role="lcghm">
                <property role="lacIc" value="# Module: " />
              </node>
            </node>
            <node concept="lc7rE" id="FTG_M003" role="3cqZAp">
              <node concept="l9hG8" id="FTG_M004" role="lcghm">
                <node concept="2OqwBi" id="FTG_M005" role="lb14g">
                  <node concept="117lpO" id="FTG_M006" role="2Oq$k0" />
                  <node concept="3TrcHB" id="FTG_M007" role="2OqNvi">
                    <ref role="3TsBF5" to="tpck:h0TrG11" resolve="name" />
                  </node>
                </node>
              </node>
            </node>
            <node concept="lc7rE" id="FTG_M008" role="3cqZAp">
              <node concept="l8MVK" id="FTG_M009" role="lcghm" />
            </node>
            <node concept="2Gpval" id="FTG_M010" role="3cqZAp">
              <node concept="2GrKxI" id="FTG_M011" role="2Gsz3X">
                <property role="TrG5h" value="pvar" />
              </node>
              <node concept="2OqwBi" id="FTG_M012" role="2GsD0m">
                <node concept="117lpO" id="FTG_M013" role="2Oq$k0" />
                <node concept="3Tsc0h" id="FTG_M014" role="2OqNvi">
                  <ref role="3TtcxE" to="k8se:7kypvuIwECG" resolve="variables" />
                </node>
              </node>
              <node concept="3clFbS" id="FTG_M015" role="2LFqv$">
                <node concept="lc7rE" id="FTG_M016" role="3cqZAp">
                  <node concept="l9hG8" id="FTG_M017" role="lcghm">
                    <node concept="2GrUjf" id="FTG_M018" role="lb14g">
                      <ref role="2Gs0qQ" node="FTG_M011" resolve="pvar" />
                    </node>
                  </node>
                </node>
                <node concept="lc7rE" id="FTG_M019" role="3cqZAp">
                  <node concept="l8MVK" id="FTG_M020" role="lcghm" />
                </node>
              </node>
            </node>
            <node concept="lc7rE" id="FTG_M040" role="3cqZAp">
              <node concept="l8MVK" id="FTG_M041" role="lcghm" />
            </node>
            <node concept="2Gpval" id="FTG_M021" role="3cqZAp">
              <node concept="2GrKxI" id="FTG_M022" role="2Gsz3X">
                <property role="TrG5h" value="pfunc" />
              </node>
              <node concept="2OqwBi" id="FTG_M023" role="2GsD0m">
                <node concept="117lpO" id="FTG_M024" role="2Oq$k0" />
                <node concept="3Tsc0h" id="FTG_M025" role="2OqNvi">
                  <ref role="3TtcxE" to="k8se:7kypvuIwECF" resolve="functions" />
                </node>
              </node>
              <node concept="3clFbS" id="FTG_M026" role="2LFqv$">
                <node concept="lc7rE" id="FTG_M027" role="3cqZAp">
                  <node concept="l9hG8" id="FTG_M028" role="lcghm">
                    <node concept="2GrUjf" id="FTG_M029" role="lb14g">
                      <ref role="2Gs0qQ" node="FTG_M022" resolve="pfunc" />
                    </node>
                  </node>
                </node>
                <node concept="lc7rE" id="FTG_M030" role="3cqZAp">
                  <node concept="l8MVK" id="FTG_M031" role="lcghm" />
                </node>
              </node>
            </node>
          </node>
          <node concept="3y3z36" id="6LZhwXWa02p" role="3clFbw">
            <node concept="2OqwBi" id="IF_PY004" role="3uHU7B">
              <node concept="117lpO" id="IF_PY005" role="2Oq$k0" />
              <node concept="3TrcHB" id="IF_PY006" role="2OqNvi">
                <ref role="3TsBF5" to="k8se:7kypvuIwECI" resolve="targetLanguage" />
              </node>
            </node>
            <node concept="Xl_RD" id="6LZhwXWavEO" role="3uHU7w">
              <property role="Xl_RC" value="cpp" />
            </node>
          </node>
        </node>
        <node concept="3clFbJ" id="IF_CPP001" role="3cqZAp">
          <node concept="3clFbS" id="IF_CPP002" role="3clFbx">
            <node concept="lc7rE" id="FTG_C000" role="3cqZAp">
              <node concept="l8MVK" id="FTG_C000b" role="lcghm" />
            </node>
            <node concept="lc7rE" id="FTG_C001" role="3cqZAp">
              <node concept="la8eA" id="FTG_C002" role="lcghm">
                <property role="lacIc" value="// Module: " />
              </node>
            </node>
            <node concept="lc7rE" id="FTG_C003" role="3cqZAp">
              <node concept="l9hG8" id="FTG_C004" role="lcghm">
                <node concept="2OqwBi" id="FTG_C005" role="lb14g">
                  <node concept="117lpO" id="FTG_C006" role="2Oq$k0" />
                  <node concept="3TrcHB" id="FTG_C007" role="2OqNvi">
                    <ref role="3TsBF5" to="tpck:h0TrG11" resolve="name" />
                  </node>
                </node>
              </node>
            </node>
            <node concept="lc7rE" id="FTG_C008" role="3cqZAp">
              <node concept="l8MVK" id="FTG_C009" role="lcghm" />
            </node>
            <node concept="2Gpval" id="FTG_C010" role="3cqZAp">
              <node concept="2GrKxI" id="FTG_C011" role="2Gsz3X">
                <property role="TrG5h" value="cvar" />
              </node>
              <node concept="2OqwBi" id="FTG_C012" role="2GsD0m">
                <node concept="117lpO" id="FTG_C013" role="2Oq$k0" />
                <node concept="3Tsc0h" id="FTG_C014" role="2OqNvi">
                  <ref role="3TtcxE" to="k8se:7kypvuIwECG" resolve="variables" />
                </node>
              </node>
              <node concept="3clFbS" id="FTG_C015" role="2LFqv$">
                <node concept="lc7rE" id="FTG_C016" role="3cqZAp">
                  <node concept="l9hG8" id="FTG_C017" role="lcghm">
                    <node concept="2OqwBi" id="FTG_C018" role="lb14g">
                      <node concept="2GrUjf" id="FTG_C019" role="2Oq$k0">
                        <ref role="2Gs0qQ" node="FTG_C011" resolve="cvar" />
                      </node>
                      <node concept="3TrEf2" id="FTG_C020" role="2OqNvi">
                        <ref role="3Tt5mk" to="k8se:7kypvuIwEFF" resolve="type" />
                      </node>
                    </node>
                  </node>
                </node>
                <node concept="lc7rE" id="FTG_C021" role="3cqZAp">
                  <node concept="la8eA" id="FTG_C022" role="lcghm">
                    <property role="lacIc" value=" " />
                  </node>
                </node>
                <node concept="lc7rE" id="FTG_C023" role="3cqZAp">
                  <node concept="l9hG8" id="FTG_C024" role="lcghm">
                    <node concept="2OqwBi" id="FTG_C025" role="lb14g">
                      <node concept="2GrUjf" id="FTG_C026" role="2Oq$k0">
                        <ref role="2Gs0qQ" node="FTG_C011" resolve="cvar" />
                      </node>
                      <node concept="3TrcHB" id="FTG_C027" role="2OqNvi">
                        <ref role="3TsBF5" to="tpck:h0TrG11" resolve="name" />
                      </node>
                    </node>
                  </node>
                </node>
                <node concept="lc7rE" id="FTG_C028" role="3cqZAp">
                  <node concept="la8eA" id="FTG_C029" role="lcghm">
                    <property role="lacIc" value=";" />
                  </node>
                </node>
                <node concept="lc7rE" id="FTG_C030" role="3cqZAp">
                  <node concept="l8MVK" id="FTG_C031" role="lcghm" />
                </node>
              </node>
            </node>
            <node concept="lc7rE" id="FTG_C040" role="3cqZAp">
              <node concept="l8MVK" id="FTG_C041" role="lcghm" />
            </node>
            <node concept="2Gpval" id="FTG_C050" role="3cqZAp">
              <node concept="2GrKxI" id="FTG_C051" role="2Gsz3X">
                <property role="TrG5h" value="cfunc" />
              </node>
              <node concept="2OqwBi" id="FTG_C052" role="2GsD0m">
                <node concept="117lpO" id="FTG_C053" role="2Oq$k0" />
                <node concept="3Tsc0h" id="FTG_C054" role="2OqNvi">
                  <ref role="3TtcxE" to="k8se:7kypvuIwECF" resolve="functions" />
                </node>
              </node>
              <node concept="3clFbS" id="FTG_C055" role="2LFqv$">
                <node concept="lc7rE" id="FTG_C056" role="3cqZAp">
                  <node concept="l9hG8" id="FTG_C057" role="lcghm">
                    <node concept="2OqwBi" id="FTG_C058" role="lb14g">
                      <node concept="2GrUjf" id="FTG_C059" role="2Oq$k0">
                        <ref role="2Gs0qQ" node="FTG_C051" resolve="cfunc" />
                      </node>
                      <node concept="3TrEf2" id="FTG_C060" role="2OqNvi">
                        <ref role="3Tt5mk" to="k8se:7kypvuIwEDG" resolve="returnType" />
                      </node>
                    </node>
                  </node>
                </node>
                <node concept="lc7rE" id="FTG_C061" role="3cqZAp">
                  <node concept="la8eA" id="FTG_C062" role="lcghm">
                    <property role="lacIc" value=" " />
                  </node>
                </node>
                <node concept="lc7rE" id="FTG_C063" role="3cqZAp">
                  <node concept="l9hG8" id="FTG_C064" role="lcghm">
                    <node concept="2OqwBi" id="FTG_C065" role="lb14g">
                      <node concept="2GrUjf" id="FTG_C066" role="2Oq$k0">
                        <ref role="2Gs0qQ" node="FTG_C051" resolve="cfunc" />
                      </node>
                      <node concept="3TrcHB" id="FTG_C067" role="2OqNvi">
                        <ref role="3TsBF5" to="tpck:h0TrG11" resolve="name" />
                      </node>
                    </node>
                  </node>
                </node>
                <node concept="lc7rE" id="FTG_C068" role="3cqZAp">
                  <node concept="la8eA" id="FTG_C069" role="lcghm">
                    <property role="lacIc" value="(" />
                  </node>
                </node>
                <node concept="2Gpval" id="FTG_C070" role="3cqZAp">
                  <node concept="2GrKxI" id="FTG_C071" role="2Gsz3X">
                    <property role="TrG5h" value="cparam" />
                  </node>
                  <node concept="2OqwBi" id="FTG_C072" role="2GsD0m">
                    <node concept="2GrUjf" id="FTG_C073" role="2Oq$k0">
                      <ref role="2Gs0qQ" node="FTG_C051" resolve="cfunc" />
                    </node>
                    <node concept="3Tsc0h" id="FTG_C074" role="2OqNvi">
                      <ref role="3TtcxE" to="k8se:7kypvuIwEDI" resolve="parameters" />
                    </node>
                  </node>
                  <node concept="3clFbS" id="FTG_C075" role="2LFqv$">
                    <node concept="lc7rE" id="FTG_C076" role="3cqZAp">
                      <node concept="l9hG8" id="FTG_C077" role="lcghm">
                        <node concept="2OqwBi" id="FTG_C078" role="lb14g">
                          <node concept="2GrUjf" id="FTG_C079" role="2Oq$k0">
                            <ref role="2Gs0qQ" node="FTG_C071" resolve="cparam" />
                          </node>
                          <node concept="3TrEf2" id="FTG_C080" role="2OqNvi">
                            <ref role="3Tt5mk" to="k8se:7kypvuIwEEE" resolve="type" />
                          </node>
                        </node>
                      </node>
                    </node>
                    <node concept="lc7rE" id="FTG_C081" role="3cqZAp">
                      <node concept="la8eA" id="FTG_C082" role="lcghm">
                        <property role="lacIc" value=" " />
                      </node>
                    </node>
                    <node concept="lc7rE" id="FTG_C083" role="3cqZAp">
                      <node concept="l9hG8" id="FTG_C084" role="lcghm">
                        <node concept="2OqwBi" id="FTG_C085" role="lb14g">
                          <node concept="2GrUjf" id="FTG_C086" role="2Oq$k0">
                            <ref role="2Gs0qQ" node="FTG_C071" resolve="cparam" />
                          </node>
                          <node concept="3TrcHB" id="FTG_C087" role="2OqNvi">
                            <ref role="3TsBF5" to="tpck:h0TrG11" resolve="name" />
                          </node>
                        </node>
                      </node>
                    </node>
                    <node concept="lc7rE" id="FTG_C088" role="3cqZAp">
                      <node concept="la8eA" id="FTG_C089" role="lcghm">
                        <property role="lacIc" value=", " />
                      </node>
                    </node>
                  </node>
                </node>
                <node concept="lc7rE" id="FTG_C090" role="3cqZAp">
                  <node concept="la8eA" id="FTG_C091" role="lcghm">
                    <property role="lacIc" value=") {" />
                  </node>
                </node>
                <node concept="lc7rE" id="FTG_C092" role="3cqZAp">
                  <node concept="l8MVK" id="FTG_C093" role="lcghm" />
                </node>
                <node concept="2Gpval" id="FTG_C100" role="3cqZAp">
                  <node concept="2GrKxI" id="FTG_C101" role="2Gsz3X">
                    <property role="TrG5h" value="cstmt" />
                  </node>
                  <node concept="2OqwBi" id="FTG_C102" role="2GsD0m">
                    <node concept="2GrUjf" id="FTG_C103" role="2Oq$k0">
                      <ref role="2Gs0qQ" node="FTG_C051" resolve="cfunc" />
                    </node>
                    <node concept="3Tsc0h" id="FTG_C104" role="2OqNvi">
                      <ref role="3TtcxE" to="k8se:7kypvuIwEDH" resolve="body" />
                    </node>
                  </node>
                  <node concept="3clFbS" id="FTG_C105" role="2LFqv$">
                    <node concept="lc7rE" id="FTG_C106" role="3cqZAp">
                      <node concept="la8eA" id="FTG_C107" role="lcghm">
                        <property role="lacIc" value="    " />
                      </node>
                    </node>
                    <node concept="lc7rE" id="FTG_C108" role="3cqZAp">
                      <node concept="l9hG8" id="FTG_C109" role="lcghm">
                        <node concept="2GrUjf" id="FTG_C110" role="lb14g">
                          <ref role="2Gs0qQ" node="FTG_C101" resolve="cstmt" />
                        </node>
                      </node>
                    </node>
                    <node concept="lc7rE" id="FTG_C111" role="3cqZAp">
                      <node concept="la8eA" id="FTG_C112" role="lcghm">
                        <property role="lacIc" value=";" />
                      </node>
                    </node>
                    <node concept="lc7rE" id="FTG_C113" role="3cqZAp">
                      <node concept="l8MVK" id="FTG_C114" role="lcghm" />
                    </node>
                  </node>
                </node>
                <node concept="lc7rE" id="FTG_C120" role="3cqZAp">
                  <node concept="la8eA" id="FTG_C121" role="lcghm">
                    <property role="lacIc" value="}" />
                  </node>
                </node>
                <node concept="lc7rE" id="FTG_C122" role="3cqZAp">
                  <node concept="l8MVK" id="FTG_C123" role="lcghm" />
                </node>
                <node concept="lc7rE" id="FTG_C124" role="3cqZAp">
                  <node concept="l8MVK" id="FTG_C125" role="lcghm" />
                </node>
              </node>
            </node>
          </node>
          <node concept="3y3z36" id="IF_CPP003" role="3clFbw">
            <node concept="2OqwBi" id="IF_CPP004" role="3uHU7B">
              <node concept="117lpO" id="IF_CPP005" role="2Oq$k0" />
              <node concept="3TrcHB" id="IF_CPP006" role="2OqNvi">
                <ref role="3TsBF5" to="k8se:7kypvuIwECI" resolve="targetLanguage" />
              </node>
            </node>
            <node concept="Xl_RD" id="IF_CPP007" role="3uHU7w">
              <property role="Xl_RC" value="python" />
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20118">
    <ref role="WuzLi" to="k8se:7kypvuIwCHC" resolve="NullLiteral" />
    <node concept="11bSqf" id="TG_20119" role="11c4hB">
      <node concept="3clFbS" id="TG_20120" role="2VODD2">
        <node concept="lc7rE" id="TG_20121" role="3cqZAp">
          <node concept="la8eA" id="TG_20122" role="lcghm">
            <property role="lacIc" value="null" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20123">
    <ref role="WuzLi" to="k8se:7kypvuIwFDD" resolve="OptimizationLock" />
    <node concept="11bSqf" id="TG_20124" role="11c4hB">
      <node concept="3clFbS" id="TG_20125" role="2VODD2">
        <node concept="lc7rE" id="TG_20126" role="3cqZAp">
          <node concept="la8eA" id="TG_20127" role="lcghm">
            <property role="lacIc" value="@lock([lockedBy], [lockReason])" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20128">
    <ref role="WuzLi" to="k8se:7kypvuIwBFC" resolve="OptionalType" />
    <node concept="11bSqf" id="TG_20129" role="11c4hB">
      <node concept="3clFbS" id="TG_20130" role="2VODD2">
        <node concept="lc7rE" id="TG_20131" role="3cqZAp">
          <node concept="la8eA" id="TG_20132" role="lcghm">
            <property role="lacIc" value="optional[[innerType]]" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20133">
    <ref role="WuzLi" to="k8se:7kypvuIwEEC" resolve="Parameter" />
    <node concept="11bSqf" id="TG_20134" role="11c4hB">
      <node concept="3clFbS" id="TG_20135" role="2VODD2">
        <node concept="lc7rE" id="FTG_P001" role="3cqZAp">
          <node concept="l9hG8" id="FTG_P002" role="lcghm">
            <node concept="2OqwBi" id="FTG_P003" role="lb14g">
              <node concept="117lpO" id="FTG_P004" role="2Oq$k0" />
              <node concept="3TrcHB" id="FTG_P005" role="2OqNvi">
                <ref role="3TsBF5" to="tpck:h0TrG11" resolve="name" />
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20138">
    <ref role="WuzLi" to="k8se:7kypvuIwBCC" resolve="PrimitiveType" />
    <node concept="11bSqf" id="TG_20139" role="11c4hB">
      <node concept="3clFbS" id="TG_20140" role="2VODD2">
        <node concept="lc7rE" id="FTG_PT001" role="3cqZAp">
          <node concept="l9hG8" id="FTG_PT002" role="lcghm">
            <node concept="2OqwBi" id="FTG_PT003" role="lb14g">
              <node concept="117lpO" id="FTG_PT004" role="2Oq$k0" />
              <node concept="3TrcHB" id="FTG_PT005" role="2OqNvi">
                <ref role="3TsBF5" to="k8se:7kypvuIwBCE" resolve="kind" />
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20143">
    <ref role="WuzLi" to="k8se:7kypvuIwDIC" resolve="Return" />
    <node concept="11bSqf" id="TG_20144" role="11c4hB">
      <node concept="3clFbS" id="TG_20145" role="2VODD2">
        <node concept="lc7rE" id="FTG_R001" role="3cqZAp">
          <node concept="la8eA" id="FTG_R002" role="lcghm">
            <property role="lacIc" value="return " />
          </node>
        </node>
        <node concept="lc7rE" id="FTG2_R001" role="3cqZAp">
          <node concept="l9hG8" id="FTG2_R002" role="lcghm">
            <node concept="2OqwBi" id="FTG2_R003" role="lb14g">
              <node concept="117lpO" id="FTG2_R004" role="2Oq$k0" />
              <node concept="3TrEf2" id="FTG2_R005" role="2OqNvi">
                <ref role="3Tt5mk" to="k8se:7kypvuIwDID" resolve="value" />
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20148">
    <ref role="WuzLi" to="k8se:7kypvuIwBGC" resolve="SetType" />
    <node concept="11bSqf" id="TG_20149" role="11c4hB">
      <node concept="3clFbS" id="TG_20150" role="2VODD2">
        <node concept="lc7rE" id="TG_20151" role="3cqZAp">
          <node concept="la8eA" id="TG_20152" role="lcghm">
            <property role="lacIc" value="set[[elementType]]" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20159">
    <ref role="WuzLi" to="k8se:7kypvuIwCFC" resolve="StringLiteral" />
    <node concept="11bSqf" id="TG_20160" role="11c4hB">
      <node concept="3clFbS" id="TG_20161" role="2VODD2">
        <node concept="lc7rE" id="TG_20162" role="3cqZAp">
          <node concept="la8eA" id="TG_20163" role="lcghm">
            <property role="lacIc" value="&quot;[value]&quot;" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20164">
    <ref role="WuzLi" to="k8se:7kypvuIwBHC" resolve="TupleType" />
    <node concept="11bSqf" id="TG_20165" role="11c4hB">
      <node concept="3clFbS" id="TG_20166" role="2VODD2">
        <node concept="lc7rE" id="TG_20167" role="3cqZAp">
          <node concept="la8eA" id="TG_20168" role="lcghm">
            <property role="lacIc" value="tuple[[elementTypes]]" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20175">
    <ref role="WuzLi" to="k8se:7kypvuIwCJC" resolve="UnaryOperation" />
    <node concept="11bSqf" id="TG_20176" role="11c4hB">
      <node concept="3clFbS" id="TG_20177" role="2VODD2">
        <node concept="lc7rE" id="TG_20178" role="3cqZAp">
          <node concept="la8eA" id="TG_20179" role="lcghm">
            <property role="lacIc" value="[operator][operand]" />
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20180">
    <ref role="WuzLi" to="k8se:7kypvuIwEFC" resolve="Variable" />
    <node concept="11bSqf" id="TG_20181" role="11c4hB">
      <node concept="3clFbS" id="TG_20182" role="2VODD2">
        <node concept="lc7rE" id="FTG_V001" role="3cqZAp">
          <node concept="l9hG8" id="FTG_V002" role="lcghm">
            <node concept="2OqwBi" id="FTG_V003" role="lb14g">
              <node concept="117lpO" id="FTG_V004" role="2Oq$k0" />
              <node concept="3TrcHB" id="FTG_V005" role="2OqNvi">
                <ref role="3TsBF5" to="tpck:h0TrG11" resolve="name" />
              </node>
            </node>
          </node>
        </node>
        <node concept="lc7rE" id="FTG_V006" role="3cqZAp">
          <node concept="la8eA" id="FTG_V007" role="lcghm">
            <property role="lacIc" value=": " />
          </node>
        </node>
        <node concept="lc7rE" id="FTG_V008" role="3cqZAp">
          <node concept="l9hG8" id="FTG_V009" role="lcghm">
            <node concept="2OqwBi" id="FTG_V010" role="lb14g">
              <node concept="117lpO" id="FTG_V011" role="2Oq$k0" />
              <node concept="3TrEf2" id="FTG_V012" role="2OqNvi">
                <ref role="3Tt5mk" to="k8se:7kypvuIwEFF" resolve="type" />
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20185">
    <ref role="WuzLi" to="k8se:7kypvuIwCKC" resolve="VariableReference" />
    <node concept="11bSqf" id="TG_20186" role="11c4hB">
      <node concept="3clFbS" id="TG_20187" role="2VODD2">
        <node concept="lc7rE" id="FTG_VR001" role="3cqZAp">
          <node concept="l9hG8" id="FTG_VR002" role="lcghm">
            <node concept="2OqwBi" id="FTG_VR003" role="lb14g">
              <node concept="117lpO" id="FTG_VR004" role="2Oq$k0" />
              <node concept="3TrcHB" id="FTG_VR005" role="2OqNvi">
                <ref role="3TsBF5" to="k8se:7kypvuIwCKD" resolve="variableName" />
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
  <node concept="WtQ9Q" id="TG_20190">
    <ref role="WuzLi" to="k8se:7kypvuIwDGC" resolve="WhileLoop" />
    <node concept="11bSqf" id="TG_20191" role="11c4hB">
      <node concept="3clFbS" id="TG_20192" role="2VODD2">
        <node concept="lc7rE" id="TG_20193" role="3cqZAp">
          <node concept="la8eA" id="TG_20194" role="lcghm">
            <property role="lacIc" value="while [condition]:&#10;[body]&#10;" />
          </node>
        </node>
      </node>
    </node>
  </node>
</model>

