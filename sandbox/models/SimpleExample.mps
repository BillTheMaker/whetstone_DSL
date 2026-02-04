<?xml version="1.0" encoding="UTF-8"?>
<model ref="r:b3d2e5a1-6f9c-482a-9d1c-a5f2c8e1b4a6(SimpleExample)">
  <persistence version="9" />
  <languages>
    <use id="c72da2b9-7cce-4447-8389-f407dc1158b7" name="jetbrains.mps.lang.structure" version="12" />
    <use id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core" version="11" />
  </languages>
  <imports>
    <import index="k8se" ref="r:e209a700-5f8c-468c-afcc-f38277628970(SemAnno.structure)" implicit="true" />
  </imports>
  <registry />
  <node concept="Module" id="TE_00001">
    <property role="name" value="MathLibrary" />
    <node concept="Variable" id="TE_00002" role="variables">
      <property role="name" value="PI" />
      <node concept="Type" id="TE_00003" role="type">
        <node concept="PrimitiveType" id="TE_00004" role="primitiveType">
          <property role="name" value="float" />
        </node>
      </node>
    </node>
    <node concept="Variable" id="TE_00005" role="variables">
      <property role="name" value="E" />
      <node concept="Type" id="TE_00006" role="type">
        <node concept="PrimitiveType" id="TE_00007" role="primitiveType">
          <property role="name" value="float" />
        </node>
      </node>
    </node>
    <node concept="Function" id="TE_00008" role="functions">
      <property role="name" value="add" />
      <node concept="Parameter" id="TE_00009" role="parameters">
        <property role="name" value="a" />
        <node concept="Type" id="TE_00010" role="type">
          <node concept="PrimitiveType" id="TE_00011" role="primitiveType">
            <property role="name" value="int" />
          </node>
        </node>
      </node>
      <node concept="Parameter" id="TE_00012" role="parameters">
        <property role="name" value="b" />
        <node concept="Type" id="TE_00013" role="type">
          <node concept="PrimitiveType" id="TE_00014" role="primitiveType">
            <property role="name" value="int" />
          </node>
        </node>
      </node>
      <node concept="Type" id="TE_00015" role="returnType">
        <node concept="PrimitiveType" id="TE_00016" role="primitiveType">
          <property role="name" value="int" />
        </node>
      </node>
      <node concept="Block" id="TE_00017" role="body">
        <node concept="Assignment" id="TE_00018" role="statements">
          <property role="name" value="result" />
          <node concept="Expression" id="TE_00019" role="value">
            <node concept="BinaryOperation" id="TE_00020" role="binaryOp">
              <property role="operator" value="+" />
              <node concept="VariableReference" id="TE_00021" role="left">
                <property role="name" value="a" />
              </node>
              <node concept="VariableReference" id="TE_00022" role="right">
                <property role="name" value="b" />
              </node>
            </node>
          </node>
        </node>
        <node concept="Return" id="TE_00023" role="statements">
          <node concept="VariableReference" id="TE_00024" role="value">
            <property role="name" value="result" />
          </node>
        </node>
      </node>
    </node>
  </node>
</model>
