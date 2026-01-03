#!/usr/bin/env python3
"""
Script para otimizar a imagem base_template.png para o ESP32
Redimensiona para 320x240 e comprime o PNG
"""

from PIL import Image
import os
import sys

def optimize_image(input_path, output_path, max_width=320, max_height=240, quality='high'):
    """
    Redimensiona e otimiza uma imagem PNG para o ESP32 mantendo a proporção original
    
    Args:
        input_path: Caminho da imagem original
        output_path: Caminho da imagem otimizada
        max_width: Largura máxima (padrão: 320)
        max_height: Altura máxima (padrão: 240)
        quality: 'high', 'medium', 'low' - qualidade da compressão
    """
    try:
        # Abrir a imagem original
        print(f"Abrindo imagem: {input_path}")
        img = Image.open(input_path)
        
        # Converter para RGB se necessário (remove transparência que pode causar problemas)
        if img.mode in ('RGBA', 'LA', 'P'):
            print("Convertendo para RGB (removendo transparência)...")
            background = Image.new('RGB', img.size, (255, 255, 255))
            if img.mode == 'P':
                img = img.convert('RGBA')
            background.paste(img, mask=img.split()[-1] if img.mode == 'RGBA' else None)
            img = background
        elif img.mode != 'RGB':
            img = img.convert('RGB')
        
        original_size = os.path.getsize(input_path)
        original_width, original_height = img.size
        
        print(f"Tamanho original: {original_width}x{original_height} pixels")
        print(f"Tamanho do arquivo: {original_size / 1024:.2f} KB")
        print(f"Proporção original: {original_width/original_height:.3f}")
        
        # Calcular novo tamanho mantendo proporção
        ratio = original_width / original_height
        
        # Calcular dimensões que cabem no espaço disponível
        if original_width / max_width > original_height / max_height:
            # A imagem é mais larga - limitar pela largura
            new_width = max_width
            new_height = int(max_width / ratio)
        else:
            # A imagem é mais alta - limitar pela altura
            new_height = max_height
            new_width = int(max_height * ratio)
        
        print(f"\nRedimensionando mantendo proporção para {new_width}x{new_height}...")
        print(f"(Máximo disponível: {max_width}x{max_height})")
        # Usar LANCZOS para melhor qualidade de redimensionamento
        img_resized = img.resize((new_width, new_height), Image.Resampling.LANCZOS)
        
        # Configurar qualidade de compressão
        compress_level = 6  # Compressão média (melhor qualidade)
        if quality == 'high':
            compress_level = 1  # Mínima compressão (melhor qualidade)
        elif quality == 'medium':
            compress_level = 6
        else:  # low
            compress_level = 9  # Máxima compressão
        
        # Salvar como PNG com melhor qualidade
        print(f"Salvando imagem otimizada com qualidade '{quality}' (compress_level={compress_level})...")
        img_resized.save(
            output_path,
            "PNG",
            optimize=False,  # Desabilitar otimização adicional para melhor qualidade
            compress_level=compress_level
        )
        
        new_size = os.path.getsize(output_path)
        reduction = ((original_size - new_size) / original_size) * 100
        
        print(f"\n✓ Imagem otimizada com sucesso!")
        print(f"Tamanho novo: {new_width}x{new_height} pixels")
        print(f"Proporção mantida: {new_width/new_height:.3f}")
        print(f"Tamanho do arquivo: {new_size / 1024:.2f} KB")
        print(f"Redução: {reduction:.1f}%")
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
    input_file = "base_template.png"
    output_file = "base_template_small.png"
    
    # Verificar se o arquivo existe
    if not os.path.exists(input_file):
        print(f"Erro: Arquivo {input_file} não encontrado!")
        print("Certifique-se de executar o script no diretório correto.")
        sys.exit(1)
    
    # Otimizar imagem com alta qualidade
    success = optimize_image(input_file, output_file, quality='high')
    
    if success:
        print("\n" + "="*50)
        print("Próximos passos:")
        print("1. Faça upload de 'base_template_small.png' para o GitHub")
        print("2. Atualize a URL no código Arduino para usar a versão pequena")
        print("="*50)
    else:
        sys.exit(1)

