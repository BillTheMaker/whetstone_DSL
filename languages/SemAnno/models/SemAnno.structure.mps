<?xml version="1.0" encoding="UTF-8"?>
<model ref="r:e209a700-5f8c-468c-afcc-f38277628970(SemAnno.structure)">
  <persistence version="9" />
  <languages>
    <devkit ref="78434eb8-b0e5-444b-850d-e7c4ad2da9ab(jetbrains.mps.devkit.aspect.structure)" />
  </languages>
  <imports>
    <import index="tpck" ref="r:00000000-0000-4000-0000-011c89590288(jetbrains.mps.lang.core.structure)" implicit="true" />
  </imports>
  <registry>
    <language id="c72da2b9-7cce-4447-8389-f407dc1158b7" name="jetbrains.mps.lang.structure">
      <concept id="1169125787135" name="jetbrains.mps.lang.structure.structure.AbstractConceptDeclaration" flags="ig" index="PkWjJ">
        <property id="6714410169261853888" name="conceptId" index="EcuMT" />
        <child id="1071489727083" name="linkDeclaration" index="1TKVEi" />
        <child id="1071489727084" name="propertyDeclaration" index="1TKVEl" />
      </concept>
      <concept id="1169127622168" name="jetbrains.mps.lang.structure.structure.InterfaceConceptReference" flags="ig" index="PrWs8">
        <reference id="1169127628841" name="intfc" index="PrY4T" />
      </concept>
      <concept id="1071489090640" name="jetbrains.mps.lang.structure.structure.ConceptDeclaration" flags="ig" index="1TIwiD">
        <property id="1096454100552" name="rootable" index="19KtqR" />
        <reference id="1071489389519" name="extends" index="1TJDcQ" />
        <child id="1169129564478" name="implements" index="PzmwI" />
      </concept>
      <concept id="1071489288299" name="jetbrains.mps.lang.structure.structure.PropertyDeclaration" flags="ig" index="1TJgyi">
        <property id="241647608299431129" name="propertyId" index="IQ2nx" />
        <reference id="1082985295845" name="dataType" index="AX2Wp" />
      </concept>
      <concept id="1071489288298" name="jetbrains.mps.lang.structure.structure.LinkDeclaration" flags="ig" index="1TJgyj">
        <property id="1071599776563" name="role" index="20kJfa" />
        <property id="1071599893252" name="sourceCardinality" index="20lbJX" />
        <property id="1071599937831" name="metaClass" index="20lmBu" />
        <property id="241647608299431140" name="linkId" index="IQ2ns" />
        <reference id="1071599976176" name="target" index="20lvS9" />
      </concept>
    </language>
    <language id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core">
      <concept id="1169194658468" name="jetbrains.mps.lang.core.structure.INamedConcept" flags="ngI" index="TrEIO">
        <property id="1169194664001" name="name" index="TrG5h" />
      </concept>
    </language>
  </registry>
  <node concept="1TIwiD" id="7_JEctSCX2J">
    <property role="EcuMT" value="8750398175680123055" />
    <property role="TrG5h" value="AgentCommand" />
    <property role="19KtqR" value="true" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
    <node concept="PrWs8" id="7_JEctSCX2N" role="PzmwI">
      <ref role="PrY4T" to="tpck:h0TrEE$" resolve="INamedConcept" />
    </node>
    <node concept="1TJgyj" id="4K9yrbkuVkN" role="1TKVEi">
      <property role="IQ2ns" value="5479061822834980147" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="metrics" />
      <ref role="20lvS9" node="12Ch1YcKjFx" resolve="ExecutionMetric" />
    </node>
    <node concept="1TJgyj" id="4K9yrbkvibg" role="1TKVEi">
      <property role="IQ2ns" value="5479061822835073744" />
      <property role="20kJfa" value="target" />
      <ref role="20lvS9" node="4K9yrbkviaT" resolve="AgentTarget" />
    </node>
  </node>
  <node concept="1TIwiD" id="12Ch1YcKjFx">
    <property role="EcuMT" value="1200284202990385889" />
    <property role="TrG5h" value="ExecutionMetric" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
    <node concept="1TJgyi" id="12Ch1YcKjF$" role="1TKVEl">
      <property role="IQ2nx" value="1200284202990385892" />
      <property role="TrG5h" value="runcount" />
      <ref role="AX2Wp" to="tpck:fKAQMTA" resolve="integer" />
    </node>
  </node>
  <node concept="1TIwiD" id="4K9yrbkviaT">
    <property role="EcuMT" value="5479061822835073721" />
    <property role="TrG5h" value="AgentTarget" />
    <property role="19KtqR" value="true" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
    <node concept="PrWs8" id="4K9yrbkviaW" role="PzmwI">
      <ref role="PrY4T" to="tpck:h0TrEE$" resolve="INamedConcept" />
    </node>
  </node>
  <node concept="1TIwiD" id="4K9yrbkw5xW">
    <property role="EcuMT" value="5479061822835284092" />
    <property role="TrG5h" value="SquadMember" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
    <node concept="1TJgyj" id="4K9yrbkwcGW" role="1TKVEi">
      <property role="IQ2ns" value="5479061822835313468" />
      <property role="20kJfa" value="agent" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="4K9yrbkviaT" resolve="AgentTarget" />
    </node>
  </node>
  <node concept="1TIwiD" id="4K9yrbkw5yl">
    <property role="EcuMT" value="5479061822835284117" />
    <property role="TrG5h" value="SquadLink" />
    <property role="19KtqR" value="true" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
    <node concept="PrWs8" id="4K9yrbkw5yn" role="PzmwI">
      <ref role="PrY4T" to="tpck:h0TrEE$" resolve="INamedConcept" />
    </node>
    <node concept="1TJgyj" id="4K9yrbkw5yq" role="1TKVEi">
      <property role="IQ2ns" value="5479061822835284122" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="members" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="4K9yrbkw5xW" resolve="SquadMember" />
    </node>
  </node>
</model>

