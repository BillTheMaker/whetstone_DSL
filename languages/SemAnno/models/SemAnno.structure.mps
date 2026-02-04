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
        <property id="4628067390765956802" name="abstract" index="R5$K7" />
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
  <node concept="1TIwiD" id="CisPcOYEky">
    <property role="EcuMT" value="725769291168392482" />
    <property role="TrG5h" value="SemanticEntity" />
    <property role="19KtqR" value="true" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
    <node concept="PrWs8" id="CisPcOYEkz" role="PzmwI">
      <ref role="PrY4T" to="tpck:h0TrEE$" resolve="INamedConcept" />
    </node>
    <node concept="1TJgyj" id="CisPcOYEkA" role="1TKVEi">
      <property role="IQ2ns" value="725769291168392486" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="components" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="CisPcOYEk$" resolve="DataComponent" />
    </node>
  </node>
  <node concept="1TIwiD" id="CisPcOYEk$">
    <property role="EcuMT" value="725769291168392484" />
    <property role="TrG5h" value="DataComponent" />
    <property role="R5$K7" value="true" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
  </node>
  <node concept="1TIwiD" id="CisPcOZ8mj">
    <property role="EcuMT" value="725769291168515475" />
    <property role="TrG5h" value="StringList" />
    <ref role="1TJDcQ" node="CisPcOYEk$" resolve="DataComponent" />
    <node concept="1TJgyi" id="CisPcOZ8mk" role="1TKVEl">
      <property role="IQ2nx" value="725769291168515476" />
      <property role="TrG5h" value="values" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
  </node>
  <node concept="1TIwiD" id="CisPcOZ8mz">
    <property role="EcuMT" value="725769291168515491" />
    <property role="TrG5h" value="Iterator" />
    <ref role="1TJDcQ" node="CisPcOYEk$" resolve="DataComponent" />
    <node concept="1TJgyj" id="CisPcOZ8m$" role="1TKVEi">
      <property role="IQ2ns" value="725769291168515492" />
      <property role="20kJfa" value="target" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="CisPcOYEky" resolve="SemanticEntity" />
    </node>
  </node>
  <node concept="1TIwiD" id="CisPcP0y__">
    <property role="EcuMT" value="725769291168885093" />
    <property role="TrG5h" value="Script" />
    <property role="19KtqR" value="true" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
    <node concept="1TJgyj" id="CisPcP0y_C" role="1TKVEi">
      <property role="IQ2ns" value="725769291168885096" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="entities" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="CisPcOYEky" resolve="SemanticEntity" />
    </node>
    <node concept="PrWs8" id="CisPcP0yA5" role="PzmwI">
      <ref role="PrY4T" to="tpck:h0TrEE$" resolve="INamedConcept" />
    </node>
  </node>
  <node concept="1TIwiD" id="GMyc5g4aRq">
    <property role="EcuMT" value="806857647106076122" />
    <property role="TrG5h" value="TextLine" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
    <node concept="1TJgyi" id="GMyc5g4aV9" role="1TKVEl">
      <property role="IQ2nx" value="806857647106076361" />
      <property role="TrG5h" value="content" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
  </node>
  <node concept="1TIwiD" id="GMyc5g4aVl">
    <property role="EcuMT" value="806857647106076373" />
    <property role="TrG5h" value="PythonScript" />
    <property role="19KtqR" value="true" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
    <node concept="PrWs8" id="GMyc5g4aVo" role="PzmwI">
      <ref role="PrY4T" to="tpck:h0TrEE$" resolve="INamedConcept" />
    </node>
    <node concept="1TJgyj" id="GMyc5g4aVr" role="1TKVEi">
      <property role="IQ2ns" value="806857647106076379" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="lines" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="GMyc5g4aRq" resolve="TextLine" />
    </node>
  </node>
  <node concept="1TIwiD" id="GMyc5g4aVv">
    <property role="EcuMT" value="806857647106076383" />
    <property role="TrG5h" value="CPPScript" />
    <property role="19KtqR" value="true" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
    <node concept="PrWs8" id="GMyc5g4aVy" role="PzmwI">
      <ref role="PrY4T" to="tpck:h0TrEE$" resolve="INamedConcept" />
    </node>
    <node concept="1TJgyj" id="GMyc5g4aV_" role="1TKVEi">
      <property role="IQ2ns" value="806857647106076389" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="lines" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="GMyc5g4aRq" resolve="TextLine" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwECC">
    <property role="EcuMT" value="8440420766104857128" />
    <property role="TrG5h" value="Module" />
    <property role="19KtqR" value="true" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
    <node concept="PrWs8" id="7kypvuIwECD" role="PzmwI">
      <ref role="PrY4T" to="tpck:h0TrEE$" resolve="INamedConcept" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwECH" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632591" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="annotations" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwFCC" resolve="Annotation" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwECG" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632590" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="variables" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwEFC" resolve="Variable" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwECF" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632589" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="functions" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwEDC" resolve="Function" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwEDC">
    <property role="EcuMT" value="8440420766104857192" />
    <property role="TrG5h" value="Function" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
    <node concept="PrWs8" id="7kypvuIwEDD" role="PzmwI">
      <ref role="PrY4T" to="tpck:h0TrEE$" resolve="INamedConcept" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwEDI" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632623" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="parameters" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwEEC" resolve="Parameter" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwEDH" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632622" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="body" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwDDC" resolve="Statement" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwEDG" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632621" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="returnType" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwBBC" resolve="Type" />
    </node>
    <node concept="1TJgyj" id="4ypvuIwEDI2" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632624" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="annotations" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwFCC" resolve="Annotation" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwEEC">
    <property role="EcuMT" value="8440420766104857256" />
    <property role="TrG5h" value="Parameter" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
    <node concept="PrWs8" id="7kypvuIwEED" role="PzmwI">
      <ref role="PrY4T" to="tpck:h0TrEE$" resolve="INamedConcept" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwEEE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632653" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="type" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwBBC" resolve="Type" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwEEF" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632654" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="defaultValue" />
      <property role="20lbJX" value="fLJekj5/_0__1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwEFC">
    <property role="EcuMT" value="8982541288447632700" />
    <property role="TrG5h" value="Variable" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
    <node concept="PrWs8" id="7kypvuIwEFD" role="PzmwI">
      <ref role="PrY4T" to="tpck:h0TrEE$" resolve="INamedConcept" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwEFF" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632702" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="type" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwBBC" resolve="Type" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwEFG" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632703" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="initializer" />
      <property role="20lbJX" value="fLJekj5/_0__1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwEFH" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632704" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="annotations" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwFCC" resolve="Annotation" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwDDC">
    <property role="EcuMT" value="8982541288447632332" />
    <property role="TrG5h" value="Statement" />
    <property role="R5$K7" value="true" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
  </node>
  <node concept="1TIwiD" id="7kypvuIwDDD">
    <property role="EcuMT" value="8982541288447632333" />
    <property role="TrG5h" value="Block" />
    <ref role="1TJDcQ" node="7kypvuIwDDC" resolve="Statement" />
    <node concept="1TJgyj" id="4ypvuIwDDD2" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632334" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="statements" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwDDC" resolve="Statement" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwDEC">
    <property role="EcuMT" value="8440420766104853160" />
    <property role="TrG5h" value="Assignment" />
    <ref role="1TJDcQ" node="7kypvuIwDDC" resolve="Statement" />
    <node concept="1TJgyj" id="7kypvuIwDED" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632365" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="target" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwDEE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632366" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="value" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
    <node concept="PrWs8" id="5oiyI7TKOpV" role="PzmwI">
      <ref role="PrY4T" to="tpck:h0TrEE$" resolve="INamedConcept" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwDFC">
    <property role="EcuMT" value="8440420766104853224" />
    <property role="TrG5h" value="IfStatement" />
    <ref role="1TJDcQ" node="7kypvuIwDDC" resolve="Statement" />
    <node concept="1TJgyj" id="7kypvuIwDFD" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632397" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="condition" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwDFE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632398" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="thenBranch" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwDDC" resolve="Statement" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwDFF" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632399" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="elseBranch" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwDDC" resolve="Statement" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwDGC">
    <property role="EcuMT" value="8982541288447632428" />
    <property role="TrG5h" value="WhileLoop" />
    <ref role="1TJDcQ" node="7kypvuIwDDC" resolve="Statement" />
    <node concept="1TJgyj" id="7kypvuIwDGD" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632429" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="condition" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwDGE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632430" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="body" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwDDC" resolve="Statement" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwDHC">
    <property role="EcuMT" value="8440420766104853352" />
    <property role="TrG5h" value="ForLoop" />
    <ref role="1TJDcQ" node="7kypvuIwDDC" resolve="Statement" />
    <node concept="1TJgyi" id="7kypvuIwDHD" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632461" />
      <property role="TrG5h" value="iteratorName" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwDHE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632462" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="iterable" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwDHF" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632463" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="body" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwDDC" resolve="Statement" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwDIC">
    <property role="EcuMT" value="8982541288447632492" />
    <property role="TrG5h" value="Return" />
    <ref role="1TJDcQ" node="7kypvuIwDDC" resolve="Statement" />
    <node concept="1TJgyj" id="7kypvuIwDID" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632493" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="value" />
      <property role="20lbJX" value="fLJekj5/_0__1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwDJC">
    <property role="EcuMT" value="8440420766104853480" />
    <property role="TrG5h" value="ExpressionStatement" />
    <ref role="1TJDcQ" node="7kypvuIwDDC" resolve="Statement" />
    <node concept="1TJgyj" id="7kypvuIwDJD" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632525" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="expression" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwCIC">
    <property role="EcuMT" value="8440420766104849320" />
    <property role="TrG5h" value="Expression" />
    <property role="R5$K7" value="true" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
  </node>
  <node concept="1TIwiD" id="4ypvuIwCIC2">
    <property role="EcuMT" value="8982541288447632557" />
    <property role="TrG5h" value="BinaryOperation" />
    <ref role="1TJDcQ" node="7kypvuIwCIC" resolve="Expression" />
    <node concept="1TJgyi" id="7kypvuIwCID" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632558" />
      <property role="TrG5h" value="operator" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwCIE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632559" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="left" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwCIF" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632560" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="right" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
    <node concept="PrWs8" id="5oiyI7TKOpW" role="PzmwI">
      <ref role="PrY4T" to="tpck:h0TrEE$" resolve="INamedConcept" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwCJC">
    <property role="EcuMT" value="8982541288447632588" />
    <property role="TrG5h" value="UnaryOperation" />
    <ref role="1TJDcQ" node="7kypvuIwCIC" resolve="Expression" />
    <node concept="1TJgyi" id="7kypvuIwCJD" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632589" />
      <property role="TrG5h" value="operator" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwCJE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632590" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="operand" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwCLC">
    <property role="EcuMT" value="8440420766104849512" />
    <property role="TrG5h" value="FunctionCall" />
    <ref role="1TJDcQ" node="7kypvuIwCIC" resolve="Expression" />
    <node concept="1TJgyi" id="7kypvuIwCLD" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632621" />
      <property role="TrG5h" value="functionName" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwCLE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632622" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="arguments" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwCKC">
    <property role="EcuMT" value="8982541288447632652" />
    <property role="TrG5h" value="VariableReference" />
    <ref role="1TJDcQ" node="7kypvuIwCIC" resolve="Expression" />
    <node concept="1TJgyi" id="7kypvuIwCKD" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632653" />
      <property role="TrG5h" value="variableName" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwCDC">
    <property role="EcuMT" value="8982541288447632556" />
    <property role="TrG5h" value="IntegerLiteral" />
    <ref role="1TJDcQ" node="7kypvuIwCIC" resolve="Expression" />
    <node concept="1TJgyi" id="7kypvuIwCDD" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632557" />
      <property role="TrG5h" value="value" />
      <ref role="AX2Wp" to="tpck:fKAQMTA" resolve="integer" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwCEC">
    <property role="EcuMT" value="8982541288447632564" />
    <property role="TrG5h" value="FloatLiteral" />
    <ref role="1TJDcQ" node="7kypvuIwCIC" resolve="Expression" />
    <node concept="1TJgyi" id="7kypvuIwCED" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632565" />
      <property role="TrG5h" value="value" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwCFC">
    <property role="EcuMT" value="8982541288447632572" />
    <property role="TrG5h" value="StringLiteral" />
    <ref role="1TJDcQ" node="7kypvuIwCIC" resolve="Expression" />
    <node concept="1TJgyi" id="7kypvuIwCFD" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632573" />
      <property role="TrG5h" value="value" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwCGC">
    <property role="EcuMT" value="8982541288447632580" />
    <property role="TrG5h" value="BooleanLiteral" />
    <ref role="1TJDcQ" node="7kypvuIwCIC" resolve="Expression" />
    <node concept="1TJgyi" id="7kypvuIwCGD" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632581" />
      <property role="TrG5h" value="value" />
      <ref role="AX2Wp" to="tpck:fKAQMTB" resolve="boolean" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwCHC">
    <property role="EcuMT" value="8440420766104849256" />
    <property role="TrG5h" value="NullLiteral" />
    <ref role="1TJDcQ" node="7kypvuIwCIC" resolve="Expression" />
  </node>
  <node concept="1TIwiD" id="7kypvuIwCMC">
    <property role="EcuMT" value="8982541288447632620" />
    <property role="TrG5h" value="ListLiteral" />
    <ref role="1TJDcQ" node="7kypvuIwCIC" resolve="Expression" />
    <node concept="1TJgyj" id="7kypvuIwCMD" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632621" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="elements" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwCNC">
    <property role="EcuMT" value="8440420766104849640" />
    <property role="TrG5h" value="IndexAccess" />
    <ref role="1TJDcQ" node="7kypvuIwCIC" resolve="Expression" />
    <node concept="1TJgyj" id="7kypvuIwCND" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632653" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="target" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwCNE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632654" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="index" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwCOC">
    <property role="EcuMT" value="8982541288447632684" />
    <property role="TrG5h" value="MemberAccess" />
    <ref role="1TJDcQ" node="7kypvuIwCIC" resolve="Expression" />
    <node concept="1TJgyj" id="7kypvuIwCOE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632686" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="target" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
    <node concept="1TJgyi" id="7kypvuIwCOD" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632685" />
      <property role="TrG5h" value="memberName" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwBBC">
    <property role="EcuMT" value="8982541288447632316" />
    <property role="TrG5h" value="Type" />
    <property role="R5$K7" value="true" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
  </node>
  <node concept="1TIwiD" id="7kypvuIwBCC">
    <property role="EcuMT" value="8440420766104844840" />
    <property role="TrG5h" value="PrimitiveType" />
    <ref role="1TJDcQ" node="7kypvuIwBBC" resolve="Type" />
    <node concept="1TJgyi" id="7kypvuIwBCE" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632334" />
      <property role="TrG5h" value="kind" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwBDC">
    <property role="EcuMT" value="8982541288447632364" />
    <property role="TrG5h" value="ListType" />
    <ref role="1TJDcQ" node="7kypvuIwBBC" resolve="Type" />
    <node concept="1TJgyj" id="7kypvuIwBDE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632366" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="elementType" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwBBC" resolve="Type" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwBGC">
    <property role="EcuMT" value="8982541288447632396" />
    <property role="TrG5h" value="SetType" />
    <ref role="1TJDcQ" node="7kypvuIwBBC" resolve="Type" />
    <node concept="1TJgyj" id="7kypvuIwBGE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632398" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="elementType" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwBBC" resolve="Type" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwBEC">
    <property role="EcuMT" value="8440420766104844968" />
    <property role="TrG5h" value="MapType" />
    <ref role="1TJDcQ" node="7kypvuIwBBC" resolve="Type" />
    <node concept="1TJgyj" id="7kypvuIwBEE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632430" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="keyType" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwBBC" resolve="Type" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwBEF" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632431" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="valueType" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwBBC" resolve="Type" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwBHC">
    <property role="EcuMT" value="8982541288447632460" />
    <property role="TrG5h" value="TupleType" />
    <ref role="1TJDcQ" node="7kypvuIwBBC" resolve="Type" />
    <node concept="1TJgyj" id="7kypvuIwBHE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632462" />
      <property role="20lmBu" value="fLJjDmT/aggregation" />
      <property role="20kJfa" value="elementTypes" />
      <property role="20lbJX" value="fLJekj5/_0__n" />
      <ref role="20lvS9" node="7kypvuIwBBC" resolve="Type" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwBIC">
    <property role="EcuMT" value="8440420766104845224" />
    <property role="TrG5h" value="ArrayType" />
    <ref role="1TJDcQ" node="7kypvuIwBBC" resolve="Type" />
    <node concept="1TJgyj" id="7kypvuIwBIE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632494" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="elementType" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwBBC" resolve="Type" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwBIF" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632495" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="size" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwBFC">
    <property role="EcuMT" value="8982541288447632524" />
    <property role="TrG5h" value="OptionalType" />
    <ref role="1TJDcQ" node="7kypvuIwBBC" resolve="Type" />
    <node concept="1TJgyj" id="7kypvuIwBFE" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632526" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="innerType" />
      <property role="20lbJX" value="fLJekj4/_1" />
      <ref role="20lvS9" node="7kypvuIwBBC" resolve="Type" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwBJC">
    <property role="EcuMT" value="8440420766104845288" />
    <property role="TrG5h" value="CustomType" />
    <ref role="1TJDcQ" node="7kypvuIwBBC" resolve="Type" />
    <node concept="1TJgyi" id="7kypvuIwBJD" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632557" />
      <property role="TrG5h" value="typeName" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwFCC">
    <property role="EcuMT" value="8440420766104861224" />
    <property role="TrG5h" value="Annotation" />
    <property role="R5$K7" value="true" />
    <ref role="1TJDcQ" to="tpck:gw2VY9q" resolve="BaseConcept" />
  </node>
  <node concept="1TIwiD" id="7kypvuIwFCD">
    <property role="EcuMT" value="8982541288447632621" />
    <property role="TrG5h" value="DerefStrategy" />
    <ref role="1TJDcQ" node="7kypvuIwFCC" resolve="Annotation" />
    <node concept="1TJgyi" id="7kypvuIwFCE" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632622" />
      <property role="TrG5h" value="strategy" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyi" id="7kypvuIwFCF" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632623" />
      <property role="TrG5h" value="derefLocation" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyi" id="7kypvuIwFCG" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632624" />
      <property role="TrG5h" value="owner" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyj" id="7kypvuIwFCH" role="1TKVEi">
      <property role="IQ2ns" value="8982541288447632625" />
      <property role="20lmBu" value="fLJjDmT/reference" />
      <property role="20kJfa" value="derefTime" />
      <property role="20lbJX" value="fLJekj5/_0__1" />
      <ref role="20lvS9" node="7kypvuIwCIC" resolve="Expression" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwFDD">
    <property role="EcuMT" value="8982541288447632653" />
    <property role="TrG5h" value="OptimizationLock" />
    <ref role="1TJDcQ" node="7kypvuIwFCC" resolve="Annotation" />
    <node concept="1TJgyi" id="7kypvuIwFDE" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632654" />
      <property role="TrG5h" value="lockedBy" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyi" id="7kypvuIwFDF" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632655" />
      <property role="TrG5h" value="lockReason" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyi" id="7kypvuIwFDG" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632656" />
      <property role="TrG5h" value="lockLevel" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyi" id="7kypvuIwFDH" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632657" />
      <property role="TrG5h" value="affectedStrategies" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyi" id="7kypvuIwFDI" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632658" />
      <property role="TrG5h" value="timestamp" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
  </node>
  <node concept="1TIwiD" id="7kypvuIwFED">
    <property role="EcuMT" value="8982541288447632685" />
    <property role="TrG5h" value="LangSpecific" />
    <ref role="1TJDcQ" node="7kypvuIwFCC" resolve="Annotation" />
    <node concept="1TJgyi" id="7kypvuIwFEE" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632686" />
      <property role="TrG5h" value="language" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyi" id="7kypvuIwFEF" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632687" />
      <property role="TrG5h" value="idiomType" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyi" id="7kypvuIwFEG" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632688" />
      <property role="TrG5h" value="rawSyntax" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyi" id="7kypvuIwFEH" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632689" />
      <property role="TrG5h" value="semanticHint" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
    <node concept="1TJgyi" id="7kypvuIwFEI" role="1TKVEl">
      <property role="IQ2nx" value="8982541288447632690" />
      <property role="TrG5h" value="position" />
      <ref role="AX2Wp" to="tpck:fKAOsGN" resolve="string" />
    </node>
  </node>
</model>

