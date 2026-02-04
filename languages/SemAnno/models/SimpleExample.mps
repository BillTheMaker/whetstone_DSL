<?xml version="1.0" encoding="UTF-8"?>
<model ref="r:b3d2e5a1-6f9c-482a-9d1c-a5f2c8e1b4a6(SimpleExample)">
  <persistence version="9" />
  <languages>
    <use id="c72da2b9-7cce-4447-8389-f407dc1158b7" name="jetbrains.mps.lang.structure" version="12" />
    <use id="ceab5195-25ea-4f22-9b92-103b95ca8c0c" name="jetbrains.mps.lang.core" version="11" />
  </languages>
  <imports>
    <import index="k8se" ref="r:e209a700-5f8c-468c-afcc-f38277628970(SemAnno.structure)" />
  </imports>
  <registry />
  <node concept="k8se:7kypvuIwECC" id="TE_00001">
    <property role="TrG5h" value="MathLibrary" />
    <node concept="k8se:7kypvuIwEFC" id="TE_00002" role="k8se:7kypvuIwECE">
      <property role="TrG5h" value="PI" />
      <node concept="k8se:7kypvuIwBBC" id="TE_00003" role="k8se:7kypvuIwEFD">
        <node concept="k8se:7kypvuIwBCC" id="TE_00004" role="k8se:7kypvuIwBBD">
          <property role="TrG5h" value="float" />
        </node>
      </node>
    </node>
    <node concept="k8se:7kypvuIwEFC" id="TE_00005" role="k8se:7kypvuIwECE">
      <property role="TrG5h" value="E" />
      <node concept="k8se:7kypvuIwBBC" id="TE_00006" role="k8se:7kypvuIwEFD">
        <node concept="k8se:7kypvuIwBCC" id="TE_00007" role="k8se:7kypvuIwBBD">
          <property role="TrG5h" value="float" />
        </node>
      </node>
    </node>
    <node concept="k8se:7kypvuIwEDC" id="TE_00008" role="k8se:7kypvuIwECF">
      <property role="TrG5h" value="add" />
      <node concept="k8se:7kypvuIwEEC" id="TE_00009" role="k8se:7kypvuIwEDE">
        <property role="TrG5h" value="a" />
        <node concept="k8se:7kypvuIwBBC" id="TE_00010" role="k8se:7kypvuIwEDF">
          <node concept="k8se:7kypvuIwBCC" id="TE_00011" role="k8se:7kypvuIwBBD">
            <property role="TrG5h" value="int" />
          </node>
        </node>
      </node>
      <node concept="k8se:7kypvuIwEEC" id="TE_00012" role="k8se:7kypvuIwEDE">
        <property role="TrG5h" value="b" />
        <node concept="k8se:7kypvuIwBBC" id="TE_00013" role="k8se:7kypvuIwEDF">
          <node concept="k8se:7kypvuIwBCC" id="TE_00014" role="k8se:7kypvuIwBBD">
            <property role="TrG5h" value="int" />
          </node>
        </node>
      </node>
      <node concept="k8se:7kypvuIwBBC" id="TE_00015" role="k8se:7kypvuIwEDH">
        <node concept="k8se:7kypvuIwBCC" id="TE_00016" role="k8se:7kypvuIwBBD">
          <property role="TrG5h" value="int" />
        </node>
      </node>
      <node concept="k8se:7kypvuIwDDD" id="TE_00017" role="k8se:7kypvuIwEDI">
        <node concept="k8se:7kypvuIwDEC" id="TE_00018" role="k8se:7kypvuIwDDE">
          <property role="TrG5h" value="result" />
          <node concept="k8se:7kypvuIwCIC" id="TE_00019" role="k8se:7kypvuIwDED">
            <node concept="k8se:7kypvuIwCIC2" id="TE_00020" role="k8se:7kypvuIwCIE">
              <property role="TrG5h" value="+" />
              <node concept="k8se:7kypvuIwCKC" id="TE_00021" role="k8se:7kypvuIwCIF">
                <property role="TrG5h" value="a" />
              </node>
              <node concept="k8se:7kypvuIwCKC" id="TE_00022" role="k8se:7kypvuIwCIG">
                <property role="TrG5h" value="b" />
              </node>
            </node>
          </node>
        </node>
        <node concept="k8se:7kypvuIwDIC" id="TE_00023" role="k8se:7kypvuIwDDE">
          <node concept="k8se:7kypvuIwCKC" id="TE_00024" role="k8se:7kypvuIwDID">
            <property role="TrG5h" value="result" />
          </node>
        </node>
      </node>
    </node>
    <node concept="k8se:7kypvuIwEDC" id="TE_00025" role="k8se:7kypvuIwECF">
      <property role="TrG5h" value="isPositive" />
      <node concept="k8se:7kypvuIwEEC" id="TE_00026" role="k8se:7kypvuIwEDE">
        <property role="TrG5h" value="value" />
        <node concept="k8se:7kypvuIwBBC" id="TE_00027" role="k8se:7kypvuIwEDF">
          <node concept="k8se:7kypvuIwBCC" id="TE_00028" role="k8se:7kypvuIwBBD">
            <property role="TrG5h" value="int" />
          </node>
        </node>
      </node>
      <node concept="k8se:7kypvuIwBBC" id="TE_00029" role="k8se:7kypvuIwEDH">
        <node concept="k8se:7kypvuIwBCC" id="TE_00030" role="k8se:7kypvuIwBBD">
          <property role="TrG5h" value="bool" />
        </node>
      </node>
      <node concept="k8se:7kypvuIwDDD" id="TE_00031" role="k8se:7kypvuIwEDI">
        <node concept="k8se:7kypvuIwDFC" id="TE_00032" role="k8se:7kypvuIwDDE">
          <node concept="k8se:7kypvuIwCIC2" id="TE_00033" role="k8se:7kypvuIwDFD">
            <property role="TrG5h" value=">" />
            <node concept="k8se:7kypvuIwCKC" id="TE_00034" role="k8se:7kypvuIwCIF">
              <property role="TrG5h" value="value" />
            </node>
            <node concept="k8se:7kypvuIwCDC" id="TE_00035" role="k8se:7kypvuIwCIG">
              <property role="TrG5h" value="0" />
            </node>
          </node>
          <node concept="k8se:7kypvuIwDDD" id="TE_00036" role="k8se:7kypvuIwDFE">
            <node concept="k8se:7kypvuIwDIC" id="TE_00037" role="k8se:7kypvuIwDDE">
              <node concept="k8se:7kypvuIwCGC" id="TE_00038" role="k8se:7kypvuIwDID">
                <property role="TrG5h" value="true" />
              </node>
            </node>
          </node>
          <node concept="k8se:7kypvuIwDDD" id="TE_00039" role="k8se:7kypvuIwDFF">
            <node concept="k8se:7kypvuIwDIC" id="TE_00040" role="k8se:7kypvuIwDDE">
              <node concept="k8se:7kypvuIwCGC" id="TE_00041" role="k8se:7kypvuIwDID">
                <property role="TrG5h" value="false" />
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
    <node concept="k8se:7kypvuIwEDC" id="TE_00042" role="k8se:7kypvuIwECF">
      <property role="TrG5h" value="factorial" />
      <node concept="k8se:7kypvuIwEEC" id="TE_00043" role="k8se:7kypvuIwEDE">
        <property role="TrG5h" value="n" />
        <node concept="k8se:7kypvuIwBBC" id="TE_00044" role="k8se:7kypvuIwEDF">
          <node concept="k8se:7kypvuIwBCC" id="TE_00045" role="k8se:7kypvuIwBBD">
            <property role="TrG5h" value="int" />
          </node>
        </node>
      </node>
      <node concept="k8se:7kypvuIwBBC" id="TE_00046" role="k8se:7kypvuIwEDH">
        <node concept="k8se:7kypvuIwBCC" id="TE_00047" role="k8se:7kypvuIwBBD">
          <property role="TrG5h" value="int" />
        </node>
      </node>
      <node concept="k8se:7kypvuIwDDD" id="TE_00048" role="k8se:7kypvuIwEDI">
        <node concept="k8se:7kypvuIwDFC" id="TE_00049" role="k8se:7kypvuIwDDE">
          <node concept="k8se:7kypvuIwCIC2" id="TE_00050" role="k8se:7kypvuIwDFD">
            <property role="TrG5h" value="<=" />
            <node concept="k8se:7kypvuIwCKC" id="TE_00051" role="k8se:7kypvuIwCIF">
              <property role="TrG5h" value="n" />
            </node>
            <node concept="k8se:7kypvuIwCDC" id="TE_00052" role="k8se:7kypvuIwCIG">
              <property role="TrG5h" value="1" />
            </node>
          </node>
          <node concept="k8se:7kypvuIwDDD" id="TE_00053" role="k8se:7kypvuIwDFE">
            <node concept="k8se:7kypvuIwDIC" id="TE_00054" role="k8se:7kypvuIwDDE">
              <node concept="k8se:7kypvuIwCDC" id="TE_00055" role="k8se:7kypvuIwDID">
                <property role="TrG5h" value="1" />
              </node>
            </node>
          </node>
        </node>
        <node concept="k8se:7kypvuIwDGC" id="TE_00056" role="k8se:7kypvuIwDDE">
          <property role="TrG5h" value="i" />
          <node concept="k8se:7kypvuIwCDC" id="TE_00057" role="k8se:7kypvuIwDGB">
            <property role="TrG5h" value="1" />
          </node>
          <node concept="k8se:7kypvuIwCIC2" id="TE_00058" role="k8se:7kypvuIwDGC">
            <property role="TrG5h" value="<" />
            <node concept="k8se:7kypvuIwCKC" id="TE_00059" role="k8se:7kypvuIwCIF">
              <property role="TrG5h" value="i" />
            </node>
            <node concept="k8se:7kypvuIwCKC" id="TE_00060" role="k8se:7kypvuIwCIG">
              <property role="TrG5h" value="n" />
            </node>
          </node>
          <node concept="k8se:7kypvuIwDDD" id="TE_00061" role="k8se:7kypvuIwDGD">
            <node concept="k8se:7kypvuIwDEC" id="TE_00062" role="k8se:7kypvuIwDDE">
              <property role="TrG5h" value="result" />
              <node concept="k8se:7kypvuIwCIC2" id="TE_00063" role="k8se:7kypvuIwDED">
                <property role="TrG5h" value="*" />
                <node concept="k8se:7kypvuIwCKC" id="TE_00064" role="k8se:7kypvuIwCIF">
                  <property role="TrG5h" value="result" />
                </node>
                <node concept="k8se:7kypvuIwCKC" id="TE_00065" role="k8se:7kypvuIwCIG">
                  <property role="TrG5h" value="i" />
                </node>
              </node>
            </node>
          </node>
        </node>
        <node concept="k8se:7kypvuIwDIC" id="TE_00066" role="k8se:7kypvuIwDDE">
          <node concept="k8se:7kypvuIwCKC" id="TE_00067" role="k8se:7kypvuIwDID">
            <property role="TrG5h" value="result" />
          </node>
        </node>
      </node>
    </node>
    <node concept="k8se:7kypvuIwEDC" id="TE_00068" role="k8se:7kypvuIwECF">
      <property role="TrG5h" value="processArray" />
      <node concept="k8se:7kypvuIwEEC" id="TE_00069" role="k8se:7kypvuIwEDE">
        <property role="TrG5h" value="items" />
        <node concept="k8se:7kypvuIwBBC" id="TE_00070" role="k8se:7kypvuIwEDF">
          <node concept="k8se:7kypvuIwBDC" id="TE_00071" role="k8se:7kypvuIwBBD">
            <node concept="k8se:7kypvuIwBCC" id="TE_00072" role="k8se:7kypvuIwBDH">
              <property role="TrG5h" value="int" />
            </node>
          </node>
        </node>
      </node>
      <node concept="k8se:7kypvuIwBBC" id="TE_00073" role="k8se:7kypvuIwEDH">
        <node concept="k8se:7kypvuIwBCC" id="TE_00074" role="k8se:7kypvuIwBBD">
          <property role="TrG5h" value="void" />
        </node>
      </node>
      <node concept="k8se:7kypvuIwDDD" id="TE_00075" role="k8se:7kypvuIwEDI">
        <node concept="k8se:7kypvuIwDHC" id="TE_00076" role="k8se:7kypvuIwDDE">
          <property role="TrG5h" value="item" />
          <node concept="k8se:7kypvuIwCKC" id="TE_00077" role="k8se:7kypvuIwDHE">
            <property role="TrG5h" value="items" />
          </node>
          <node concept="k8se:7kypvuIwDDD" id="TE_00078" role="k8se:7kypvuIwDHF">
            <node concept="k8se:7kypvuIwDEC" id="TE_00079" role="k8se:7kypvuIwDDE">
              <property role="TrG5h" value="squared" />
              <node concept="k8se:7kypvuIwCIC2" id="TE_00080" role="k8se:7kypvuIwDED">
                <property role="TrG5h" value="*" />
                <node concept="k8se:7kypvuIwCKC" id="TE_00081" role="k8se:7kypvuIwCIF">
                  <property role="TrG5h" value="item" />
                </node>
                <node concept="k8se:7kypvuIwCKC" id="TE_00082" role="k8se:7kypvuIwCIG">
                  <property role="TrG5h" value="item" />
                </node>
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
    <node concept="k8se:7kypvuIwEDC" id="TE_00083" role="k8se:7kypvuIwECF">
      <property role="TrG5h" value="createPoint" />
      <node concept="k8se:7kypvuIwBBC" id="TE_00084" role="k8se:7kypvuIwEDH">
        <node concept="k8se:7kypvuIwBEC" id="TE_00085" role="k8se:7kypvuIwBBD">
          <node concept="k8se:7kypvuIwBCC" id="TE_00086" role="k8se:7kypvuIwBED">
            <property role="TrG5h" value="int" />
          </node>
          <node concept="k8se:7kypvuIwBCC" id="TE_00087" role="k8se:7kypvuIwBED">
            <property role="TrG5h" value="int" />
          </node>
        </node>
      </node>
      <node concept="k8se:7kypvuIwDDD" id="TE_00088" role="k8se:7kypvuIwEDI">
        <node concept="k8se:7kypvuIwDIC" id="TE_00089" role="k8se:7kypvuIwDDE">
          <node concept="k8se:7kypvuIwCMC" id="TE_00090" role="k8se:7kypvuIwDID">
            <node concept="k8se:7kypvuIwCDC" id="TE_00091" role="k8se:7kypvuIwCMD">
              <property role="TrG5h" value="10" />
            </node>
            <node concept="k8se:7kypvuIwCDC" id="TE_00092" role="k8se:7kypvuIwCMD">
              <property role="TrG5h" value="20" />
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
</model>
