#!/usr/bin/env python3
"""
Script para rotacionar a imagem base_template_small.png 90 graus
"""

from PIL import Image
import os
import sys

def rotate_image(input_path, output_path, degrees=90):
    """
    Rotaciona uma imagem PNG
    
    Args:
        input_path: Caminho da imagem original
        output_path: Caminho da imagem rotacionada
        degrees: Graus de rotação (padrão: 90)
    """
    try:
        # Abrir a imagem original
        print(f"Abrindo imagem: {input_path}")
        img = Image.open(input_path)
        
        original_size = os.path.getsize(input_path)
        original_width, original_height = img.size
        
        print(f"Tamanho original: {original_width}x{original_height} pixels")
        print(f"Tamanho do arquivo: {original_size / 1024:.2f} KB")
        
        # Rotacionar a imagem
        print(f"\nRotacionando {degrees} graus...")
        img_rotated = img.rotate(-degrees, expand=True)  # Negative for clockwise
        
        rotated_width, rotated_height = img_rotated.size
        print(f"Tamanho após rotação: {rotated_width}x{rotated_height} pixels")
        
        # Salvar como PNG otimizado
        print(f"Salvando imagem rotacionada: {output_path}")
        img_rotated.save(
            output_path,
            "PNG",
            optimize=True,
            compress_level=9  # Máxima compressão
        )
        
        new_size = os.path.getsize(output_path)
        
        print(f"\n✓ Imagem rotacionada com sucesso!")
        print(f"Tamanho do arquivo: {new_size / 1024:.2f} KB")
        print(f"\nArquivo salvo em: {output_path}")
        
        return True
        
    except FileNotFoundError:
        print(f"Erro: Arquivo não encontrado: {input_path}")
        return False
    except Exception as e:
        print(f"Erro ao processar imagem: {e}")
        return False

if __name__ == "__main__":
    # Caminhos
    input_file = "base_template_small.png"
    output_file = "base_template_small.png"  # Sobrescrever o arquivo original
    
    # Verificar se o arquivo existe
    if not os.path.exists(input_file):
        print(f"Erro: Arquivo {input_file} não encontrado!")
        print("Certifique-se de executar o script no diretório correto.")
        sys.exit(1)
    
    # Rotacionar imagem
    success = rotate_image(input_file, output_file, 90)
    
    if success:
        print("\n" + "="*50)
        print("Próximos passos:")
        print("1. Faça upload da imagem rotacionada para o GitHub")
        print("2. O ESP32 exibirá a imagem já rotacionada")
        print("="*50)
    else:
        sys.exit(1)

