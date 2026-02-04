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
  <node concept="7kypvuIwECC" id="TE_00001">
    <property role="TrG5h" value="MathLibrary" />
    <node concept="7kypvuIwEFC" id="TE_00002" role="7kypvuIwECE">
      <property role="TrG5h" value="PI" />
      <node concept="7kypvuIwBBC" id="TE_00003" role="7kypvuIwEFD">
        <node concept="7kypvuIwBCC" id="TE_00004" role="7kypvuIwBBD">
          <property role="TrG5h" value="float" />
        </node>
      </node>
    </node>
    <node concept="7kypvuIwEFC" id="TE_00005" role="7kypvuIwECE">
      <property role="TrG5h" value="E" />
      <node concept="7kypvuIwBBC" id="TE_00006" role="7kypvuIwEFD">
        <node concept="7kypvuIwBCC" id="TE_00007" role="7kypvuIwBBD">
          <property role="TrG5h" value="float" />
        </node>
      </node>
    </node>
    <node concept="7kypvuIwEDC" id="TE_00008" role="7kypvuIwECF">
      <property role="TrG5h" value="add" />
      <node concept="7kypvuIwEEC" id="TE_00009" role="7kypvuIwEDE">
        <property role="TrG5h" value="a" />
        <node concept="7kypvuIwBBC" id="TE_00010" role="7kypvuIwEDF">
          <node concept="7kypvuIwBCC" id="TE_00011" role="7kypvuIwBBD">
            <property role="TrG5h" value="int" />
          </node>
        </node>
      </node>
      <node concept="7kypvuIwEEC" id="TE_00012" role="7kypvuIwEDE">
        <property role="TrG5h" value="b" />
        <node concept="7kypvuIwBBC" id="TE_00013" role="7kypvuIwEDF">
          <node concept="7kypvuIwBCC" id="TE_00014" role="7kypvuIwBBD">
            <property role="TrG5h" value="int" />
          </node>
        </node>
      </node>
      <node concept="7kypvuIwBBC" id="TE_00015" role="7kypvuIwEDH">
        <node concept="7kypvuIwBCC" id="TE_00016" role="7kypvuIwBBD">
          <property role="TrG5h" value="int" />
        </node>
      </node>
      <node concept="7kypvuIwDDD" id="TE_00017" role="7kypvuIwEDI">
        <node concept="7kypvuIwDEC" id="TE_00018" role="7kypvuIwDDE">
          <property role="TrG5h" value="result" />
          <node concept="7kypvuIwCIC" id="TE_00019" role="7kypvuIwDED">
            <node concept="7kypvuIwCIC2" id="TE_00020" role="7kypvuIwCIE">
              <property role="TrG5h" value="+" />
              <node concept="7kypvuIwCKC" id="TE_00021" role="7kypvuIwCIF">
                <property role="TrG5h" value="a" />
              </node>
              <node concept="7kypvuIwCKC" id="TE_00022" role="7kypvuIwCIG">
                <property role="TrG5h" value="b" />
              </node>
            </node>
          </node>
        </node>
        <node concept="7kypvuIwDIC" id="TE_00023" role="7kypvuIwDDE">
          <node concept="7kypvuIwCKC" id="TE_00024" role="7kypvuIwDID">
            <property role="TrG5h" value="result" />
          </node>
        </node>
      </node>
    </node>
    <node concept="7kypvuIwEDC" id="TE_00025" role="7kypvuIwECF">
      <property role="TrG5h" value="isPositive" />
      <node concept="7kypvuIwEEC" id="TE_00026" role="7kypvuIwEDE">
        <property role="TrG5h" value="value" />
        <node concept="7kypvuIwBBC" id="TE_00027" role="7kypvuIwEDF">
          <node concept="7kypvuIwBCC" id="TE_00028" role="7kypvuIwBBD">
            <property role="TrG5h" value="int" />
          </node>
        </node>
      </node>
      <node concept="7kypvuIwBBC" id="TE_00029" role="7kypvuIwEDH">
        <node concept="7kypvuIwBCC" id="TE_00030" role="7kypvuIwBBD">
          <property role="TrG5h" value="bool" />
        </node>
      </node>
      <node concept="7kypvuIwDDD" id="TE_00031" role="7kypvuIwEDI">
        <node concept="7kypvuIwDFC" id="TE_00032" role="7kypvuIwDDE">
          <node concept="7kypvuIwCIC2" id="TE_00033" role="7kypvuIwDFD">
            <property role="TrG5h" value=">" />
            <node concept="7kypvuIwCKC" id="TE_00034" role="7kypvuIwCIF">
              <property role="TrG5h" value="value" />
            </node>
            <node concept="7kypvuIwCDC" id="TE_00035" role="7kypvuIwCIG">
              <property role="TrG5h" value="0" />
            </node>
          </node>
          <node concept="7kypvuIwDDD" id="TE_00036" role="7kypvuIwDFE">
            <node concept="7kypvuIwDIC" id="TE_00037" role="7kypvuIwDDE">
              <node concept="7kypvuIwCGC" id="TE_00038" role="7kypvuIwDID">
                <property role="TrG5h" value="true" />
              </node>
            </node>
          </node>
          <node concept="7kypvuIwDDD" id="TE_00039" role="7kypvuIwDFF">
            <node concept="7kypvuIwDIC" id="TE_00040" role="7kypvuIwDDE">
              <node concept="7kypvuIwCGC" id="TE_00041" role="7kypvuIwDID">
                <property role="TrG5h" value="false" />
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
    <node concept="7kypvuIwEDC" id="TE_00042" role="7kypvuIwECF">
      <property role="TrG5h" value="factorial" />
      <node concept="7kypvuIwEEC" id="TE_00043" role="7kypvuIwEDE">
        <property role="TrG5h" value="n" />
        <node concept="7kypvuIwBBC" id="TE_00044" role="7kypvuIwEDF">
          <node concept="7kypvuIwBCC" id="TE_00045" role="7kypvuIwBBD">
            <property role="TrG5h" value="int" />
          </node>
        </node>
      </node>
      <node concept="7kypvuIwBBC" id="TE_00046" role="7kypvuIwEDH">
        <node concept="7kypvuIwBCC" id="TE_00047" role="7kypvuIwBBD">
          <property role="TrG5h" value="int" />
        </node>
      </node>
      <node concept="7kypvuIwDDD" id="TE_00048" role="7kypvuIwEDI">
        <node concept="7kypvuIwDFC" id="TE_00049" role="7kypvuIwDDE">
          <node concept="7kypvuIwCIC2" id="TE_00050" role="7kypvuIwDFD">
            <property role="TrG5h" value="<=" />
            <node concept="7kypvuIwCKC" id="TE_00051" role="7kypvuIwCIF">
              <property role="TrG5h" value="n" />
            </node>
            <node concept="7kypvuIwCDC" id="TE_00052" role="7kypvuIwCIG">
              <property role="TrG5h" value="1" />
            </node>
          </node>
          <node concept="7kypvuIwDDD" id="TE_00053" role="7kypvuIwDFE">
            <node concept="7kypvuIwDIC" id="TE_00054" role="7kypvuIwDDE">
              <node concept="7kypvuIwCDC" id="TE_00055" role="7kypvuIwDID">
                <property role="TrG5h" value="1" />
              </node>
            </node>
          </node>
        </node>
        <node concept="7kypvuIwDGC" id="TE_00056" role="7kypvuIwDDE">
          <property role="TrG5h" value="i" />
          <node concept="7kypvuIwCDC" id="TE_00057" role="7kypvuIwDGB">
            <property role="TrG5h" value="1" />
          </node>
          <node concept="7kypvuIwCIC2" id="TE_00058" role="7kypvuIwDGC">
            <property role="TrG5h" value="<" />
            <node concept="7kypvuIwCKC" id="TE_00059" role="7kypvuIwCIF">
              <property role="TrG5h" value="i" />
            </node>
            <node concept="7kypvuIwCKC" id="TE_00060" role="7kypvuIwCIG">
              <property role="TrG5h" value="n" />
            </node>
          </node>
          <node concept="7kypvuIwDDD" id="TE_00061" role="7kypvuIwDGD">
            <node concept="7kypvuIwDEC" id="TE_00062" role="7kypvuIwDDE">
              <property role="TrG5h" value="result" />
              <node concept="7kypvuIwCIC2" id="TE_00063" role="7kypvuIwDED">
                <property role="TrG5h" value="*" />
                <node concept="7kypvuIwCKC" id="TE_00064" role="7kypvuIwCIF">
                  <property role="TrG5h" value="result" />
                </node>
                <node concept="7kypvuIwCKC" id="TE_00065" role="7kypvuIwCIG">
                  <property role="TrG5h" value="i" />
                </node>
              </node>
            </node>
          </node>
        </node>
        <node concept="7kypvuIwDIC" id="TE_00066" role="7kypvuIwDDE">
          <node concept="7kypvuIwCKC" id="TE_00067" role="7kypvuIwDID">
            <property role="TrG5h" value="result" />
          </node>
        </node>
      </node>
    </node>
    <node concept="7kypvuIwEDC" id="TE_00068" role="7kypvuIwECF">
      <property role="TrG5h" value="processArray" />
      <node concept="7kypvuIwEEC" id="TE_00069" role="7kypvuIwEDE">
        <property role="TrG5h" value="items" />
        <node concept="7kypvuIwBBC" id="TE_00070" role="7kypvuIwEDF">
          <node concept="7kypvuIwBDC" id="TE_00071" role="7kypvuIwBBD">
            <node concept="7kypvuIwBCC" id="TE_00072" role="7kypvuIwBDH">
              <property role="TrG5h" value="int" />
            </node>
          </node>
        </node>
      </node>
      <node concept="7kypvuIwBBC" id="TE_00073" role="7kypvuIwEDH">
        <node concept="7kypvuIwBCC" id="TE_00074" role="7kypvuIwBBD">
          <property role="TrG5h" value="void" />
        </node>
      </node>
      <node concept="7kypvuIwDDD" id="TE_00075" role="7kypvuIwEDI">
        <node concept="7kypvuIwDHC" id="TE_00076" role="7kypvuIwDDE">
          <property role="TrG5h" value="item" />
          <node concept="7kypvuIwCKC" id="TE_00077" role="7kypvuIwDHE">
            <property role="TrG5h" value="items" />
          </node>
          <node concept="7kypvuIwDDD" id="TE_00078" role="7kypvuIwDHF">
            <node concept="7kypvuIwDEC" id="TE_00079" role="7kypvuIwDDE">
              <property role="TrG5h" value="squared" />
              <node concept="7kypvuIwCIC2" id="TE_00080" role="7kypvuIwDED">
                <property role="TrG5h" value="*" />
                <node concept="7kypvuIwCKC" id="TE_00081" role="7kypvuIwCIF">
                  <property role="TrG5h" value="item" />
                </node>
                <node concept="7kypvuIwCKC" id="TE_00082" role="7kypvuIwCIG">
                  <property role="TrG5h" value="item" />
                </node>
              </node>
            </node>
          </node>
        </node>
      </node>
    </node>
    <node concept="7kypvuIwEDC" id="TE_00083" role="7kypvuIwECF">
      <property role="TrG5h" value="createPoint" />
      <node concept="7kypvuIwBBC" id="TE_00084" role="7kypvuIwEDH">
        <node concept="7kypvuIwBEC" id="TE_00085" role="7kypvuIwBBD">
          <node concept="7kypvuIwBCC" id="TE_00086" role="7kypvuIwBED">
            <property role="TrG5h" value="int" />
          </node>
          <node concept="7kypvuIwBCC" id="TE_00087" role="7kypvuIwBED">
            <property role="TrG5h" value="int" />
          </node>
        </node>
      </node>
      <node concept="7kypvuIwDDD" id="TE_00088" role="7kypvuIwEDI">
        <node concept="7kypvuIwDIC" id="TE_00089" role="7kypvuIwDDE">
          <node concept="7kypvuIwCMC" id="TE_00090" role="7kypvuIwDID">
            <node concept="7kypvuIwCDC" id="TE_00091" role="7kypvuIwCMD">
              <property role="TrG5h" value="10" />
            </node>
            <node concept="7kypvuIwCDC" id="TE_00092" role="7kypvuIwCMD">
              <property role="TrG5h" value="20" />
            </node>
          </node>
        </node>
      </node>
    </node>
  </node>
</model>
