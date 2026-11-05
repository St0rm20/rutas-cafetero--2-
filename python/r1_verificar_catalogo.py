"""Genera results/catalogo_mapa.html para comprobar a ojo que cada lugar cae donde debe.

El enunciado pide que las coordenadas se verifiquen en un mapa: abra el HTML, revise cada marcador
y corrija data/catalogo.csv donde haga falta (luego vuelva a ejecutar r1_preparar.py).
"""
import folium
import pandas as pd

from common import DATOS, RES, crear_carpetas

COLORES = {"Cabecera municipal": "red", "Atractivo turistico": "blue",
           "Finca cafetera u hotel rural": "green", "Centro de acopio o trilladora": "orange"}

crear_carpetas()
df = pd.read_csv(DATOS / "catalogo.csv")
m = folium.Map(location=[4.55, -75.69], zoom_start=10)
for _, r in df.iterrows():
    folium.Marker([r["lat"], r["lon"]], tooltip=f'{r["id"]} - {r["nombre"]}',
                  icon=folium.Icon(color=COLORES.get(r["categoria"], "gray"))).add_to(m)
m.save(RES / "catalogo_mapa.html")
print("Mapa guardado en results/catalogo_mapa.html")
print(df["categoria"].value_counts().to_string())
