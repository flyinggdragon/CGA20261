import math
import sys
from direct.showbase.ShowBase import ShowBase
from panda3d.core import DirectionalLight, AmbientLight, Vec4


class Esfera:
    def __init__(self, centro, raio, id_obj):
        self.centro = list(centro)  # [x, y, z]
        self.raio = raio
        self.id = id_obj
        self.colidindo = False


class AABB:
    def __init__(self, centro, dimensoes, id_obj):
        self.centro = list(centro)  # [x, y, z]
        self.dimensoes = dimensoes  # [largura, altura, profundidade]
        self.id = id_obj
        self.colidindo = False

    def obter_limites(self):
        min_x = self.centro[0] - self.dimensoes[0] / 2
        max_x = self.centro[0] + self.dimensoes[0] / 2
        min_y = self.centro[1] - self.dimensoes[1] / 2
        max_y = self.centro[1] + self.dimensoes[1] / 2
        min_z = self.centro[2] - self.dimensoes[2] / 2
        max_z = self.centro[2] + self.dimensoes[2] / 2
        return (min_x, max_x), (min_y, max_y), (min_z, max_z)


def checar_esfera_esfera(e1, e2):
    dist_quadrada = (
        (e1.centro[0] - e2.centro[0])**2 +
        (e1.centro[1] - e2.centro[1])**2 +
        (e1.centro[2] - e2.centro[2])**2
    )
    soma_raios = e1.raio + e2.raio
    return dist_quadrada <= (soma_raios ** 2)


def checar_aabb_aabb(b1, b2):
    lim1_x, lim1_y, lim1_z = b1.obter_limites()
    lim2_x, lim2_y, lim2_z = b2.obter_limites()

    colisao_x = lim1_x[1] >= lim2_x[0] and lim2_x[1] >= lim1_x[0]
    colisao_y = lim1_y[1] >= lim2_y[0] and lim2_y[1] >= lim1_y[0]
    colisao_z = lim1_z[1] >= lim2_z[0] and lim2_z[1] >= lim1_z[0]

    return colisao_x and colisao_y and colisao_z


def checar_esfera_aabb(esfera, aabb):
    lim_x, lim_y, lim_z = aabb.obter_limites()

    # Estratégia Híbrida (Clamp)
    ponto_proximo_x = max(lim_x[0], min(esfera.centro[0], lim_x[1]))
    ponto_proximo_y = max(lim_y[0], min(esfera.centro[1], lim_y[1]))
    ponto_proximo_z = max(lim_z[0], min(esfera.centro[2], lim_z[1]))

    dist_quadrada = (
        (ponto_proximo_x - esfera.centro[0])**2 +
        (ponto_proximo_y - esfera.centro[1])**2 +
        (ponto_proximo_z - esfera.centro[2])**2
    )

    return dist_quadrada <= (esfera.raio ** 2)


class ColetorColisoes3D(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)

        self.cam.setPos(0, -20, 5)
        self.cam.lookAt(0, 0, 0)
        self.configurar_iluminacao()

        self.objetos_fisicos = [
            Esfera(centro=[-3, 0, 2], raio=1.0, id_obj="Esfera_1"),
            Esfera(centro=[3, 0, 2], raio=1.0, id_obj="Esfera_2"),
            AABB(centro=[-2, 0, -2],
                 dimensoes=[1.5, 1.5, 1.5], id_obj="Caixa_1"),
            AABB(centro=[2, 0, -2],
                 dimensoes=[1.5, 1.5, 1.5], id_obj="Caixa_2")
        ]

        self.modelos_3d = {}
        self.criar_cenario_visual()

        self.indice_selecionado = 0
        self.atualizar_titulo_janela()
        self.configurar_inputs()

        self.taskMgr.add(self.loop_atualizacao, "LoopFisica")

    def configurar_iluminacao(self):
        alight = AmbientLight('alight')
        alight.setColor(Vec4(0.4, 0.4, 0.4, 1))
        alnp = self.render.attachNewNode(alight)
        self.render.setLight(alnp)

        dlight = DirectionalLight('dlight')
        dlight.setColor(Vec4(0.8, 0.8, 0.8, 1))
        dlnp = self.render.attachNewNode(dlight)
        dlnp.setHpr(45, -45, 0)
        self.render.setLight(dlnp)

    def criar_cenario_visual(self):
        for obj in self.objetos_fisicos:
            if isinstance(obj, Esfera):
                modelo = self.loader.loadModel("models/misc/sphere")
                modelo.setScale(obj.raio)
                cor_padrao = Vec4(0.2, 0.6, 1.0, 1)
            else:
                modelo = self.loader.loadModel("models/box")
                modelo.setPos(-0.5, -0.5, -0.5)
                container = self.render.attachNewNode(f"container_{obj.id}")
                modelo.reparentTo(container)
                container.setScale(
                    obj.dimensoes[0], obj.dimensoes[1], obj.dimensoes[2])
                modelo = container
                cor_padrao = Vec4(0.2, 0.8, 0.2, 1)

            modelo.reparentTo(self.render)
            modelo.setPos(obj.centro[0], obj.centro[1], obj.centro[2])
            modelo.setColor(cor_padrao)

            self.modelos_3d[obj.id] = {
                'nodepath': modelo,
                'cor_original': cor_padrao
            }

    def configurar_inputs(self):
        self.accept("1", self.mudar_selecao, [0])
        self.accept("2", self.mudar_selecao, [1])
        self.accept("3", self.mudar_selecao, [2])
        self.accept("4", self.mudar_selecao, [3])
        self.accept("escape", sys.exit)

    def mudar_selecao(self, indice):
        self.indice_selecionado = indice
        self.atualizar_titulo_janela()

    def atualizar_titulo_janela(self):
        obj = self.objetos_fisicos[self.indice_selecionado]
        print(f"\n[INFO] Controlando agora: {obj.id}")
        print("Use: W/S (Eixo Y), A/D (Eixo X), Q/E (Eixo Z)")

    def loop_atualizacao(self, task):
        dt = globalClock.getDt()
        vel = 5.0 * dt
        obj_atual = self.objetos_fisicos[self.indice_selecionado]

        is_down = self.mouseWatcherNode.is_button_down
        if is_down("d"):
            obj_atual.centro[0] += vel
        if is_down("a"):
            obj_atual.centro[0] -= vel
        if is_down("w"):
            obj_atual.centro[1] += vel
        if is_down("s"):
            obj_atual.centro[1] -= vel
        if is_down("q"):
            obj_atual.centro[2] += vel
        if is_down("e"):
            obj_atual.centro[2] -= vel

        for obj in self.objetos_fisicos:
            self.modelos_3d[obj.id]['nodepath'].setPos(
                obj.centro[0], obj.centro[1], obj.centro[2])
            obj.colidindo = False

        n = len(self.objetos_fisicos)
        for i in range(n):
            for j in range(i + 1, n):
                o1 = self.objetos_fisicos[i]
                o2 = self.objetos_fisicos[j]

                colidiu = False

                if isinstance(o1, Esfera) and isinstance(o2, Esfera):
                    colidiu = checar_esfera_esfera(o1, o2)
                elif isinstance(o1, AABB) and isinstance(o2, AABB):
                    colidiu = checar_aabb_aabb(o1, o2)
                elif isinstance(o1, Esfera) and isinstance(o2, AABB):
                    colidiu = checar_esfera_aabb(o1, o2)
                elif isinstance(o1, AABB) and isinstance(o2, Esfera):
                    colidiu = checar_esfera_aabb(o2, o1)

                if colidiu:
                    o1.colidindo = True
                    o2.colidindo = True

        for obj in self.objetos_fisicos:
            meta_visual = self.modelos_3d[obj.id]
            if obj.colidindo:
                meta_visual['nodepath'].setColor(Vec4(1, 0, 0, 1))
            else:
                meta_visual['nodepath'].setColor(meta_visual['cor_original'])

        return task.cont


if __name__ == "__main__":
    app = ColetorColisoes3D()
    app.run()
