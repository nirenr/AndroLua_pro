package com.myopicmobile.textwarrior.common;
import android.text.*;
import android.widget.*;
import android.widget.MultiAutoCompleteTextView.*;
import java.io.*;
import java.util.*;

public class AutoComplete
{
	public static int createAutoIndent(CharSequence text)
	{
		LuaLexer lexer = new LuaLexer(text);
		int idt = 0;
		try
		{
			while (true)
			{
				LuaTokenTypes type = lexer.advance();
				if (type == null)
				{
					break;
				}
				idt += indent(type);
			}
		}
		catch (IOException e)
		{
            e.printStackTrace();
        }
		return idt;
	}


	private static int indent(LuaTokenTypes t)
	{
        switch (t)
		{
		    case DO:
            case FUNCTION:
            case THEN:
            case REPEAT:
			case LCURLY:
				return 1;
            case UNTIL:
			case ELSEIF:
			case END:
            case RCURLY:
			    return -1;
            default:
                return 0;
        }
    }

	public static CharSequence format(CharSequence text, int width)
	{
		StringBuilder builder=new StringBuilder();
		boolean isNewLine=true;
		LuaLexer lexer = new LuaLexer(text);
		try
		{
			int idt = 0;

			while (true)
			{
				LuaTokenTypes type = lexer.advance();
				if (type == null)
					break;
				if (type == LuaTokenTypes.NEWLINE)
				{
					isNewLine = true;
					builder.append('\n');
					idt = Math.max(0, idt);
					
				}
				else if (isNewLine)
				{
					if(type==LuaTokenTypes.WS){
						
					}
					else if (type == LuaTokenTypes.ELSE)
					{
						idt--;
						builder.append(createIntdent(idt * width));
						builder.append(lexer.yytext());
						idt++;
						isNewLine = false;
					}
					else if (type == LuaTokenTypes.ELSEIF || type == LuaTokenTypes.END || type == LuaTokenTypes.UNTIL || type == LuaTokenTypes.RCURLY)
					{
						idt--;
						builder.append(createIntdent(idt * width));
						builder.append(lexer.yytext());

						isNewLine = false;
					}
					else
					{
						builder.append(createIntdent(idt * width));
						builder.append(lexer.yytext());
						idt += indent(type);
						isNewLine = false;
					}
				}
				else if(type==LuaTokenTypes.WS){
					builder.append(' ');
				}
				else
				{
					builder.append(lexer.yytext());
					idt += indent(type);
				}

			}
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}

		return builder;
	}

	private static char[] createIntdent(int n)
	{
		if (n < 0)
			return new char[0];
		char[] idts= new char[n];
		for (int i=0;i < n;i++)
			idts[i] = ' ';
		return idts;
	}

}
