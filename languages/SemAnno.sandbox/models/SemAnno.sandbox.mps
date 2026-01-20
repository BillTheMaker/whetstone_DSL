<?xml version="1.0" encoding="UTF-8"?>
<model ref="r:7e00567c-a09a-4d4d-bc4a-644a7db85ea4(SemAnno.sandbox)">
  <persistence version="9" />
  <languages>
    <use id="a452e1cf-6f8c-4bca-8447-fd4d6e4d0003" name="SemAnno" version="0" />
  </languages>
  <imports />
  <registry>
    <language id="a452e1cf-6f8c-4bca-8447-fd4d6e4d0003" name="SemAnno">
      <concept id="8750398175680123055" name="SemAnno.structure.AgentCommand" flags="ng" index="1N7xC">
        <reference id="5479061822835073744" name="target" index="2otaHy" />
        <child id="5479061822834980147" name="metrics" index="2oszM1" />
      </concept>
      <concept id="5479061822835073721" name="SemAnno.structure.AgentTarget" flags="ng" index="2otaGb" />
      <concept id="5479061822835284117" name="SemAnno.structure.SquadLink" flags="ng" index="2oyt4B">
        <child id="5479061822835284122" name="members" index="2oyt4C" />
      </concept>
      <concept id="5479061822835284092" name="SemAnno.structure.SquadMember" flags="ng" index="2oyt7e">
        <reference id="5479061822835313468" name="agent" index="2oykae" />
      </concept>
      <concept id="1200284202990385889" name="SemAnno.structure.ExecutionMetric" flags="ng" index="2sqnVY">
        <property id="1200284202990385892" name="runcount" index="2sqnVV" />
      </concept>
    </language>
    <language id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core">
      <concept id="1169194658468" name="jetbrains.mps.lang.core.structure.INamedConcept" flags="ngI" index="TrEIO">
        <property id="1169194664001" name="name" index="TrG5h" />
      </concept>
    </language>
  </registry>
  <node concept="1N7xC" id="4zm6nSzGYJZ">
    <property role="TrG5h" value="Defend" />
  </node>
  <node concept="2otaGb" id="4K9yrbkvnV_">
    <property role="TrG5h" value="Mainframe" />
  </node>
  <node concept="1N7xC" id="4K9yrbkvnVA">
    <property role="TrG5h" value="Attack" />
    <ref role="2otaHy" node="4K9yrbkvnV_" resolve="Mainframe" />
    <node concept="2sqnVY" id="4K9yrbkw5xV" role="2oszM1">
      <property role="2sqnVV" value="0" />
    </node>
  </node>
  <node concept="2otaGb" id="4K9yrbkw0YE">
    <property role="TrG5h" value="Database2" />
  </node>
  <node concept="2oyt4B" id="4K9yrbkwcGO">
    <property role="TrG5h" value="Alpha" />
    <node concept="2oyt7e" id="4K9yrbkwcGU" role="2oyt4C">
      <ref role="2oykae" node="4K9yrbkvnV_" resolve="Mainframe" />
    </node>
    <node concept="2oyt7e" id="4K9yrbkwnjI" role="2oyt4C">
      <ref role="2oykae" node="4K9yrbkw0YE" resolve="Database2" />
    </node>
    <node concept="2oyt7e" id="4K9yrbkwnjJ" role="2oyt4C">
      <ref role="2oykae" node="4K9yrbkvnV_" resolve="Mainframe" />
    </node>
  </node>
  <node concept="2oyt4B" id="10sW45eDj3I">
    <property role="TrG5h" value="AlphaTeam" />
    <node concept="2oyt7e" id="10sW45eDj3J" role="2oyt4C">
      <ref role="2oykae" node="4K9yrbkvnV_" resolve="Mainframe" />
    </node>
    <node concept="2oyt7e" id="10sW45eDj3K" role="2oyt4C">
      <ref role="2oykae" node="4K9yrbkw0YE" resolve="Database2" />
    </node>
  </node>
</model>

